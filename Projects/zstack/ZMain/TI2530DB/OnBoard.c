/**************************************************************************************************
  Filename:       OnBoard.c
  Revised:        $Date: 2009-12-16 17:44:49 -0800 (Wed, 16 Dec 2009) $
  Revision:       $Revision: 21351 $

  Description:    This file contains the UI and control for the
                  peripherals on the EVAL development board
  Notes:          This file targets the Chipcon CC2530


  Copyright 2005-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_SYS.h"
#include "DebugTrace.h"

/* Hal */
#include "hal_lcd.h"
#include "hal_mcu.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_led.h"

/* Allow access macRandomByte() */
#include "mac_radio_defs.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Task ID not initialized
#define NO_TASK_ID 0xFF

// Minimum length RAM "pattern" for Stack check
#define MIN_RAM_INIT 12

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8 OnboardKeyIntEnable;

#if defined MAKE_CRC_SHDW
#pragma location="CRC_SHDW"
const CODE uint16 _crcShdw = 0xFFFF;
#pragma required=_crcShdw
#endif

// 64-bit Extended Address of this device
uint8 aExtendedAddress[8];

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Registered keys task ID, initialized to NOT USED.
static uint8 registeredKeysTaskID = NO_TASK_ID; // 0xff

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void ChkReset( void );

/*********************************************************************
 * @fn      InitBoard()
 * @brief   Initialize the CC2420DB Board Peripherals
 * @param   level: COLD,WARM,READY
 * @return  None
 */
void InitBoard( uint8 level )
{
  if ( level == OB_COLD )  // OB_COLD = 0，如果设备为冷启动，即上电复位启动
  {
    // Interrupts off
    osal_int_disable( INTS_ALL );
    // Turn all LEDs off
    HalLedSet( HAL_LED_ALL, HAL_LED_MODE_OFF );
    // Check for Brown-Out reset
    ChkReset(); // 检测上一次复位的类型，并处理
  }
  else  // !OB_COLD
  {
    /* Initialize Key stuff */
    OnboardKeyIntEnable = HAL_KEY_INTERRUPT_DISABLE;    // 0x00
    HalKeyConfig( OnboardKeyIntEnable, OnBoard_KeyCallback);// 回调函数始终在 hal_key.c 中的 HalKeyPoll()中被调用
  }
}

/*********************************************************************
 * @fn      ChkReset()
 * @brief   Check reset bits - if reset cause is unknown, assume a
 *          brown-out (low power), assume batteries are not reliable,
 *          hang in a loop and sequence through the LEDs.
 * @param   None
 * @return  None
 *********************************************************************/
void ChkReset( void )
{
  uint8 led;
  uint8 rib;

  // Isolate reset indicator bits
  rib = SLEEPSTA & LRESET;

  if ( rib == RESETPO )
  {
    // Put code here to handle Power-On reset
  }
  else if ( rib == RESETEX )
  {
    // Put code here to handle External reset
  }
  else if ( rib == RESETWD )
  {
    // Put code here to handle WatchDog reset
#pragma message("看门狗复位")
  }
  else
  {
    // Unknown, hang and blink
    HAL_DISABLE_INTERRUPTS();
    led = HAL_LED_4;
    while ( 1 ) {
      HalLedSet( led, HAL_LED_MODE_ON );
      MicroWait( 62500 );
      MicroWait( 62500 );
      HalLedSet( led, HAL_LED_MODE_OFF );
      MicroWait( 37500 );
      MicroWait( 37500 );
      if ( !(led >>= 1) )
        led = HAL_LED_4;
    }
  }
}

/*********************************************************************
 *                        "Keyboard" Support
 *********************************************************************/

/*********************************************************************
 * Keyboard Register function
 *
 * The keyboard handler is setup to send all keyboard changes to
 * one task (if a task is registered).
 *
 * If a task registers, it will get all the keys. You can change this
 * to register for individual keys.
 *********************************************************************/
uint8 RegisterForKeys( uint8 task_id )
{
  // Allow only the first task
  if ( registeredKeysTaskID == NO_TASK_ID ) // 0xff
  {
    registeredKeysTaskID = task_id; // task_id = sapi_TaskID
    return ( true );
  }
  else
    return ( false );
}

/*********************************************************************
 * @fn      OnBoard_SendKeys
 *
 * @brief   Send "Key Pressed" message to application.
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  status
 *********************************************************************/
uint8 OnBoard_SendKeys( uint8 keys, uint8 state ) 
{
  keyChange_t *msgPtr;

  // 默认 registeredKeysTaskID = NO_TASK_ID, 如需使用，则需调用
  // RegisterForKeys(task_id) 函数注册按键事件到 某层（如 APS 层）中，
  // 然后在该层中的事件处理函数中处理按键事件
  if ( registeredKeysTaskID != NO_TASK_ID )
  {
    // Send the address to the task
    msgPtr = (keyChange_t *)osal_msg_allocate( sizeof(keyChange_t) );
    if ( msgPtr )
    {
      msgPtr->hdr.event = KEY_CHANGE;
      msgPtr->state = state; // ture if HAL_KEY_SW_6(P0_1), or false
      msgPtr->keys = keys;

      // 将按键事件捆绑到系统消息中，再将系统消息添加到消息队列中，
      // 然后发送系统消息事件（SYS_EVENT_MSG）到 registeredKeysTaskID
      // 任务标识 ID 所属的层（如 APS 层）中，
      // 该层在事件处理函数中处理系统消息事件，而系统消息中又包含
      // 有按键事件。
      osal_msg_send( registeredKeysTaskID, (uint8 *)msgPtr );
    }
    return ( ZSuccess );
  }
  else
    return ( ZFailure );
}

/*********************************************************************
 * @fn      OnBoard_KeyCallback
 *
 * @brief   Callback service for keys
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  void
 *********************************************************************/
// 在 hal_key.c 中的 HalKeyPoll() 中被调用
void OnBoard_KeyCallback ( uint8 keys, uint8 state )
{
  uint8 shift;
  (void)state;

  /* Get shift key status */
  shift = ((keys & HAL_KEY_SW_6) ? true : false);   // S1 = P0_1

  // 如果已向 某层（如 APS 层）注册了按键事件，则返回 ZSuccess，
  // 此时就在该层中处理按键事件；
  // 否则返回 ZFailure，此时在此处理按键事件
  if ( OnBoard_SendKeys( keys, shift ) != ZSuccess )
  {
    // Process SW1 here
    if ( keys & HAL_KEY_SW_1 )  // Switch 1
    {
    }
    // Process SW2 here
    if ( keys & HAL_KEY_SW_2 )  // Switch 2
    {
    }
    // Process SW3 here
    if ( keys & HAL_KEY_SW_3 )  // Switch 3
    {
    }
    // Process SW4 here
    if ( keys & HAL_KEY_SW_4 )  // Switch 4
    {
    }
    // Process SW5 here
    if ( keys & HAL_KEY_SW_5 )  // Switch 5
    {
    }
    // Process SW6 here
    if ( keys & HAL_KEY_SW_6 )  // Switch 6
    {
    }
  }
}

/*********************************************************************
 * @fn      OnBoard_stack_used
 *
 * @brief   Runs through the stack looking for touched memory.
 *
 * @param   none
 *
 * @return  Maximum number of bytes used by the stack.
 *********************************************************************/
uint16 OnBoard_stack_used(void)
{
  uint8 const *ptr;
  uint8 cnt = 0;

  for (ptr = XSTACK_END; ptr > XSTACK_BEG; ptr--)
  {
    if (STACK_INIT_VALUE == *ptr)
    {
      if (++cnt >= MIN_RAM_INIT)
      {
        ptr += MIN_RAM_INIT;
        break;
      }
    }
    else
    {
      cnt = 0;
    }
  }

  return (uint16)(XSTACK_END - ptr + 1);
}

/*********************************************************************
 * @fn      _itoa
 *
 * @brief   convert a 16bit number to ASCII
 *
 * @param   num -
 *          buf -
 *          radix -
 *
 * @return  void
 *
 *********************************************************************/
void _itoa(uint16 num, uint8 *buf, uint8 radix)
{
  char c,i;
  uint8 *p, rst[5];

  p = rst;
  for ( i=0; i<5; i++,p++ )
  {
    c = num % radix;  // Isolate a digit
    *p = c + (( c < 10 ) ? '0' : '7');  // Convert to Ascii
    num /= radix;
    if ( !num )
      break;
  }

  for ( c=0 ; c<=i; c++ )
    *buf++ = *p--;  // Reverse character order

  *buf = '\0';
}

/*********************************************************************
 * @fn        Onboard_rand
 *
 * @brief    Random number generator
 *
 * @param   none
 *
 * @return  uint16 - new random number
 *
 *********************************************************************/
uint16 Onboard_rand( void )
{
  return ( MAC_RADIO_RANDOM_WORD() );
}

/*********************************************************************
 * @fn        Onboard_wait
 *
 * @brief    Delay wait
 *
 * @param   uint16 - time to wait
 *
 * @return  none
 *
 *********************************************************************/
void Onboard_wait( uint16 timeout )
{
  while (timeout--)
  {
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }
}

/*********************************************************************
 *                    EXTERNAL I/O FUNCTIONS
 *
 * User defined functions to control external devices. Add your code
 * to the following functions to control devices wired to DB outputs.
 *
 *********************************************************************/

void BigLight_On( void )
{
  // Put code here to turn on an external light
}

void BigLight_Off( void )
{
  // Put code here to turn off an external light
}

void BuzzerControl( uint8 on )
{
  // Put code here to turn a buzzer on/off
  (void)on;
}

void Dimmer( uint8 lvl )
{
  // Put code here to control a dimmer
  (void)lvl;
}

// No dip switches on this board
uint8 GetUserDipSw( void )
{
  return 0;
}

/*********************************************************************
*********************************************************************/
