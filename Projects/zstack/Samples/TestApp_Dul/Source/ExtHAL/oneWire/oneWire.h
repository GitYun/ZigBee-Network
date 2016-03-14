/**
   *******************************************************************
   * @file		oneWire.h
   * @author	RQY
   * @version	V0.1
   * @date		2016-01-30
   * @brief		这个文件包含单总线器件库的所有函数原型
   *******************************************************************
   */

/* Define to prevent recursive inclusion ----------------------------*/
#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ---------------------------------------------------------*/
#include "iOCC2530.h"
	
#ifndef owTRUE
	#define owTRUE	1		/**< function arfument value*/
#endif
	
#ifndef owFLASE
	#define owFLASE	0		/**< function arfument value*/
#endif
	
#ifndef ENABLE
	#define ENABLE	owTRUE
#endif
	
#ifndef DISABLE
	#define DISABLE	owFLASE
#endif

#ifndef owSuccess
	#define owSuccess	0		/**< function return value*/
#endif

#ifndef owFailure
	#define owFailure 1		/**< function return value*/
#endif
	
/* One-Wire Bus Define ----------------------------------------------*/
#define PERIPH_PORT		PORT
#define PERIPH_PIN		PIN
#define PORT_PIN			PX_Y
#define PxSEL					P0SEL
#define PxDIR					P0DIR
#define PxINP					P0INP

#define afio()				( PxSEL |= (1 << PERIPH_PIN) )
#define gpio()				( PxSEL &= ~(1 << PERIPH_PIN) )
#define outputMode()	( PxDIR |= (1 << PERIPH_PIN) )
#define inputMode()		( PxDIR &= ~(1 << PERIPH_PIN) )
#define outputLow()		( PORT_PIN = 0 )
#define outputHigh()	( PORT_PIN = 1 )
#define pullUp()			P2INP &= ~(0x20 << PERIPH_PORT); \
												PxINP &= ~(1 << PERIPH_PIN)
#define pullDown()		P2INP |= (0x20 << PERIPH_PORT); \
												PxINP &= ~(1 << PERIPH_PIN)
#define readPin()			( PORT_PIN )

/* one Wire Bus Gobal Variable --------------------------------------*/
extern unsigned char PORT;
extern unsigned char PIN;
extern unsigned char PX_Y;

/* One Wire Bus Functions -------------------------------------------*/
void oneWireReset(unsigned int pullLowTime, unsigned int pullHighTime);
unsigned char oneWireCheck(unsigned int waitRspTime, unsigned char isSlavePullHigh,
													 unsigned char isChangeToInputMode);
void oneWireWrite(unsigned char data);
unsigned char oneWireRead(unsigned char isLSB, unsigned char isSlaveOutputReadTimeSlot);

void configOneWire(unsigned char port, unsigned char pin, unsigned char px_y);
void delay_us(unsigned int us);
void delay_ms(unsigned int ms);

#ifdef __cplusplus
}
#endif

#endif