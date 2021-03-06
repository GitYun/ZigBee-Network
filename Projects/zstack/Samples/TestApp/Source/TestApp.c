/**************************************************************************************************
  Filename:       TestApp.c
  Revised:        $Date: 2009-03-18 15:56:27 -0700 (Wed, 18 Mar 2009) $
  Revision:       $Revision: 19453 $

  Description:    Generic Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

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
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends "Hello World" to another "Generic"
  application every 15 seconds.  The application will also
  receive "Hello World" packets.

  The "Hello World" messages are sent/received as MSG type message.

  This applications doesn't have a profile, so it handles everything
  directly - itself.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "aps_groups.h"

#include "TestApp.h"
#include "DebugTrace.h"

#if !defined( WIN32 )
  #include "OnBoard.h"
#endif

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"

#include "mt_uart.h"

#include "stdlib.h"
#include "string.h"

#if defined ( DHT11 )
	#include "dht11.h"
#endif
#if defined ( DS18B20 )
	#include "ds18b20.h"
#endif


/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 Dht11Data[6];

// This list should be filled with Application specific Cluster IDs.
const cId_t TestApp_ClusterList[TESTAPP_MAX_CLUSTERS] =
{
  TESTAPP_CLUSTERID,
	TESTAPP_PERIODIC_CLUSTERID
};

const SimpleDescriptionFormat_t TestApp_SimpleDesc =
{
  TESTAPP_ENDPOINT,              //  int Endpoint;
  TESTAPP_PROFID,                //  uint16 AppProfId[2];
  TESTAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  TESTAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  TESTAPP_FLAGS,                 //  int   AppFlags:4;
  TESTAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)TestApp_ClusterList,  //  byte *pAppInClusterList;
  TESTAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)TestApp_ClusterList   //  byte *pAppOutClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in TestApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t TestApp_epDesc;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
byte TestApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // TestApp_Init() is called.
devStates_t TestApp_NwkState;


byte TestApp_TransID;  // This is the unique message ID (counter)

afAddrType_t TestApp_DstAddr;						// 用于单播
afAddrType_t TestApp_Periodic_DstAddr;	// 用于周期性广播
afAddrType_t TestApp_Flash_DstAddr;			// 用于组播

aps_Group_t TestApp_Group;	//  用于组播

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void TestApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
void TestApp_HandleKeys( byte shift, byte keys );
void TestApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void TestApp_SendTheMessage( void );
void TestApp_SendPeriodicDHT11Message(void);
void TestApp_SendPeriodicDS18B20Message(void);
uint8 TestApp_GetDHT11SensorData(void);

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      TestApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void TestApp_Init( byte task_id )
{
  TestApp_TaskID = task_id;
  TestApp_NwkState = DEV_INIT;
  TestApp_TransID = 0;
	
	// By default, all devices start out in Group 1
	// Initialization Group -- Group ID and name
	TestApp_Group.ID = 0x0003;
	osal_memcpy( TestApp_Group.name, "Group 3", 7 );
	// Add endpoint to the Group on the task
	// 在本任务里将端点加入到组中
	aps_AddGroup( TESTAPP_ENDPOINT, &TestApp_Group );
	
  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  MT_UartInit();
  MT_UartRegisterTaskID(task_id);
  HalUARTWrite(0, "UartInit OK\n", sizeof("UartInit OK\n"));
  
  
	// TestApp_DstAddr 的类型为 afAddrType_t，通过该结构体指定的地址将通过 OTA 层 
	// 直接发送至指定的设备，主要是用于数据发送时对地址的要求
	
	// AddrNotPresent 按照绑定的方式进行单播
  TestApp_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  TestApp_DstAddr.endPoint = 0;
  TestApp_DstAddr.addr.shortAddr = 0;
	
	// AddrBroadcast 广播给所有设备，包括睡眠的设备
	TestApp_Periodic_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
	TestApp_Periodic_DstAddr.endPoint = TESTAPP_ENDPOINT;
	TestApp_Periodic_DstAddr.addr.shortAddr = 0xFFFF;
	TestApp_Periodic_DstAddr.panId = ZDAPP_CONFIG_PAN_ID;	// 0xFFFB
	
	// Setup for the flash command's destination address - Group 1
	// afAddrGroup 组播给具有相同 Group ID 的设备
	TestApp_Flash_DstAddr.addrMode = (afAddrMode_t)afAddrGroup;
	TestApp_Flash_DstAddr.endPoint = TESTAPP_ENDPOINT;
	TestApp_Flash_DstAddr.addr.shortAddr = TestApp_Group.ID;	// Group ID = 3

  // Fill out the endpoint description.
  TestApp_epDesc.endPoint = TESTAPP_ENDPOINT;
  TestApp_epDesc.task_id = &TestApp_TaskID;
  TestApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&TestApp_SimpleDesc;
  TestApp_epDesc.latencyReq = noLatencyReqs;

  // Register the endpoint description with the AF
  afRegister( &TestApp_epDesc );

  // Register for all key events - This app will handle all key events
  RegisterForKeys( TestApp_TaskID );

  // Update the display
#if defined ( LCD_SUPPORTED )
    HalLcdWriteString( "TestApp", HAL_LCD_LINE_1 );
#endif
    
//  ZDO_RegisterForZDOMsg( TestApp_TaskID, End_Device_Bind_rsp );
//  ZDO_RegisterForZDOMsg( TestApp_TaskID, Match_Desc_rsp );
}

/*********************************************************************
 * @fn      TestApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
UINT16 TestApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;

  // Data Confirmation message fields
  byte sentEP;
  ZStatus_t sentStatus;
  byte sentTransID;       // This should match the value sent
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    while ( MSGpkt )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZDO_CB_MSG:
          TestApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;
          
        case KEY_CHANGE:
          TestApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            // The data wasn't delivered -- Do something
          }
          break;

        case AF_INCOMING_MSG_CMD:
          TestApp_MessageMSGCB( MSGpkt );
          break;

        case ZDO_STATE_CHANGE:
          TestApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
//          if ( (TestApp_NwkState == DEV_ZB_COORD)
//              || (TestApp_NwkState == DEV_ROUTER)
//              || (TestApp_NwkState == DEV_END_DEVICE) )
//          {
//            // Start sending "the" message in a regular interval.
//            osal_start_timerEx( TestApp_TaskID,
//                                TESTAPP_SEND_MSG_EVT,
//                              TESTAPP_SEND_MSG_TIMEOUT );
//          }
            if ( TestApp_NwkState == DEV_END_DEVICE )
            {
                osal_start_timerEx( TestApp_TaskID, 
                                    TESTAPP_SEND_PERIODIC_MSG_EVT, 
                                    50 );// 50ms
								//osal_set_event(TestAPP_TaskId, TESTAPP_SEND_PERIODIC_MSG_EVT);
            }
					
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( TestApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  // Send a message out - This event is generated by a timer
  //  (setup in TestApp_Init()).
  if ( events & TESTAPP_SEND_MSG_EVT )
  {
    // Send "the" message
    TestApp_SendTheMessage();

    // Setup to send message again
    osal_start_timerEx( TestApp_TaskID,
                        TESTAPP_SEND_MSG_EVT,
                      TESTAPP_SEND_MSG_TIMEOUT );

    // return unprocessed events
    return (events ^ TESTAPP_SEND_MSG_EVT);
  }
	
	if ( events & TESTAPP_SEND_PERIODIC_MSG_EVT )
	{
#if defined ( DHT11 )		
		TestApp_SendPeriodicDHT11Message();
#endif
#if defined ( DS18B20 )
		TestApp_SendPeriodicDS18B20Message();
#endif
		
		osal_start_timerEx( TestApp_TaskID,
												TESTAPP_SEND_PERIODIC_MSG_EVT,
												40 );	// 40ms
		
		return ( events ^ TESTAPP_SEND_PERIODIC_MSG_EVT );
	}

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      TestApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void TestApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined(BLINK_LEDS)
      else
      {
        // Flash LED to show failure
        HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            TestApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            TestApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            TestApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}

/*********************************************************************
 * @fn      TestApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void TestApp_HandleKeys( byte shift, byte keys )
{
//  zAddrType_t dstAddr;
  
  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request for the mandatory endpoint
//      dstAddr.addrMode = Addr16Bit;
//      dstAddr.addr.shortAddr = 0x0000; // Coordinator
//      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(), 
//                            TestApp_epDesc.endPoint,
//                            TESTAPP_PROFID,
//                            TESTAPP_MAX_CLUSTERS, (cId_t *)TestApp_ClusterList,
//                            TESTAPP_MAX_CLUSTERS, (cId_t *)TestApp_ClusterList,
//                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
//      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
//      // Initiate a Match Description Request (Service Discovery)
//      dstAddr.addrMode = AddrBroadcast;
//      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
//      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
//                        TESTAPP_PROFID,
//                        TESTAPP_MAX_CLUSTERS, (cId_t *)TestApp_ClusterList,
//                        TESTAPP_MAX_CLUSTERS, (cId_t *)TestApp_ClusterList,
//                        FALSE );
    }
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      TestApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void TestApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case TESTAPP_CLUSTERID:
      // "the" message
#if defined( LCD_SUPPORTED )
      HalLcdWriteScreen( (char*)pkt->cmd.Data, "rcvd" );
#elif defined( WIN32 )
      WPRINTSTR( pkt->cmd.Data );
#endif			
      break;
      
    case TESTAPP_PERIODIC_CLUSTERID:
			if (pkt->cmd.Data[pkt->cmd.DataLength - 1] == '1')
			{
				HalUARTWrite(0, "DHT11: ", 7);
				HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength - 1);
				HalUARTWrite(0, "\r\n", 2);
			}
			if (pkt->cmd.Data[pkt->cmd.DataLength - 1] == '2')
			{
				HalUARTWrite(0, "DS18B20: ", 9);
				HalUARTWrite(0, pkt->cmd.Data, pkt->cmd.DataLength - 1);
				HalUARTWrite(0, "\r\n", 2);
			}
		break;
  }
}

/*********************************************************************
 * @fn      TestApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void TestApp_SendTheMessage( void )
{
  char theMessageData[] = "Hello World";

  if ( AF_DataRequest( &TestApp_DstAddr, &TestApp_epDesc,
                       TESTAPP_CLUSTERID,
                       (byte)osal_strlen( theMessageData ) + 1,
                       (byte *)&theMessageData,
                       &TestApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
  {
    // Successfully requested to be sent.
  }
  else
  {
    // Error occurred in request to send.
  }
}

/*********************************************************************
 * @fn      TestApp_SendPeriodicDHT11Message
 *
 * @brief   Send periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void TestApp_SendPeriodicDHT11Message( void )
{
	volatile uint8 dataLen = 0;
	dataLen = TestApp_GetDHT11SensorData();
	if ( dataLen > 0 )
	{	
		if ( AF_DataRequest( &TestApp_Periodic_DstAddr, &TestApp_epDesc,
												 TESTAPP_PERIODIC_CLUSTERID, dataLen, (byte *)Dht11Data, 
												 &TestApp_TransID, AF_DISCV_ROUTE,
												 AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
		{
//			HalLedSet(HAL_LED_2, HAL_LED_MODE_TOGGLE);
		}
		else
		{
		 
		}
	}
}

/*********************************************************************
 * @fn      TestApp_SendPeriodicDS18B20Message
 *
 * @brief   Send periodic message.
 *
 * @param   none
 *
 * @return  none
 */
void TestApp_SendPeriodicDS18B20Message()
{
#if defined ( DS18B20 )	
	uint8 dataLen = 0;
	char Ds18b20Data[6] = {0};	
	char hundreds, tens, units, decimal;	
	float temperature;
	
	temperature = dsReadT();
	
	if (temperature == -10000.0)
		return;
	hundreds = (char)(temperature / 100) + 0x30;
	tens = (char)(((int)temperature % 100) / 10 ) + 0x30;
	units = (char)((int)temperature % 10) + 0x30;
	decimal = (char)((temperature - (int)temperature) * 10) + 0x30;	
	
	if (hundreds > 0x30 && hundreds < 0x3A)
		Ds18b20Data[dataLen++] = hundreds;
	
	if (tens >= 0x30 && tens < 0x3A)
	{
		if (hundreds == 0x30 && tens != 0x30)
			Ds18b20Data[dataLen++] = tens;
		else if (hundreds != 0x30)
			Ds18b20Data[dataLen++] = tens;
	}
	
	if (units >= 0x30 && units < 0x3A)
	{
		Ds18b20Data[dataLen++] = units;
	}
	
	if (decimal > 0x30 && decimal < 0x3A)
	{
		Ds18b20Data[dataLen++] = '.';
		Ds18b20Data[dataLen++] = decimal;
	}
		
	Ds18b20Data[dataLen] = '2';
		
	if ( dataLen > 0 )
	{	
		if ( AF_DataRequest( &TestApp_Periodic_DstAddr, &TestApp_epDesc,
												 TESTAPP_PERIODIC_CLUSTERID, dataLen + 1, (byte *)Ds18b20Data, 
												 &TestApp_TransID, AF_DISCV_ROUTE,
												 AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
		{
//			HalLedSet(HAL_LED_2, HAL_LED_MODE_TOGGLE);
		}
		else
		{
		 
		}
	}	
#endif	
}




/*********************************************************************
 * @fn      TestApp_GetDHT11SensorData
 *
 * @brief   Get DHT11 sensor temperature and humidity data.
 *
 * @param   dataArray - pointer, pointer to the DHT11 sensor data buffer.
 *
 * @return none
 */
uint8 TestApp_GetDHT11SensorData()
{
#if defined ( DHT11 )
	uint8 dataLen = 0;	
	uint8 dht11[2] = {0, 0};
	static uint8 initFalg = 1;
	
	if ( initFalg )
	{
		if ( !DHT11IOInit() )	// 初始化 DHT11 的 IO 引脚
		{
			//osal_clear_event(TestApp_TaskID, TESTAPP_SEND_PERIODIC_MSG_EVT);
			// waiting for DHT11 ready
			osal_start_timerEx(TestApp_TaskID, TESTAPP_SEND_PERIODIC_MSG_EVT, 1000);// 1s			

			initFalg = 0;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if ( DHT11ReadData(&dht11[0], &dht11[1]) == 0 )
		{
			Dht11Data[dataLen++] = dht11[0] / 10 + 0x30;	// temperature's tens
			Dht11Data[dataLen++] = dht11[0] % 10 + 0x30;	// temperature's units
			
			Dht11Data[dataLen++] = 0x20;
			
			Dht11Data[dataLen++] = dht11[1] / 10 + 0x30;	// humidity's tens
			Dht11Data[dataLen++] = dht11[1] % 10 + 0x30;	// humidity's units
			
			Dht11Data[dataLen++] = '1';
			
			return dataLen;
		}
	}
#endif	
	return 0;	
}

/*********************************************************************
*********************************************************************/
