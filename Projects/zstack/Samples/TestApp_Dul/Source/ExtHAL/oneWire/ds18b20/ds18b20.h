/**
   *************************************************************************
   * @file		ds18b20.h
   * @author	RQY
   * @version	V0.1
   * @date		2016-01-30
   * @brief		这个文件包含操作DS18B20的所有寄存器和函数原型
   *************************************************************************
   */

/* Define to prevent recursive inclusion ---------------------------------*/
#ifndef __DS18B20_H
#define	__DS18B20_H

/* Includes ---------------------------------------------------------------*/
#include "oneWire.h"

/* DS18B20 Temperature Output Accuracy -------------------------------------*/
#define HALF			0
#define QUARTER		1
#define EIGHTH		2
#define SIXTEENTH	3
#define DEFAULT		3					/**< 默认输出精度为0.0625，即SIXTEENTH*/

/* DS18B20 Function Commands -----------------------------------------------*/
#define	CONVERT_T		0x44		/**< Initiates temperature conversion.*/

#define READ_REG		0xBE		/**< Reads the entire scratchpad including the CRC byte.*/

#define WRITE_REG		0x4E		/**< Writes data into scratchpad bytes 2,3, 
															and 4(TH, TL, and configuration registers).*/

#define COPY_REG		0x48		/**< Copies TH, TL, and configuration register data
													 		from the scratchpad to EEPROM.*/

#define RECALL_E2		0xB8		/**< Recalls TH, TL, and configuration register data
													 		from EEPROM to the scratchpad.*/

#define READ_POWER	0xB4		/**< Signals DS18B20 power supply mode to the master.*/


/* DS18B20 ROM Commands --------------------------------------------------------*/
#define SEARCH_ROM	0xF0		/**< The master identify the ROM codes of all slave devices on the bus.*/

#define READ_ROM		0x33		/**< The master to read the slave's 64-bit ROM
															code without using the Search ROM procedure.
															This commmand can only be used when there is one slave on the bus.*/

#define MATCH_ROM		0x55		/**< The match ROM commmand followed by a 64-bit
													 		ROM code sequence allows the bus master to
													 		address a specific slave device on a multidrop or single-drop bus.*/

#define SKIP_ROM		0xCC		/**< The master can use this command to address all
														 	devices on the bus simultaneously without sending 
														 	out any ROM code information.*/

#define ALARM_SEARCH 0xEC

/* DS18B20 Gobal Variable -----------------------------------------------------*/
extern unsigned char preAccuracyVlaue;

/* DS18B20 Functions Prototype-------------------------------------------------*/
unsigned char dsInit(void);
float dsReadT(unsigned char outputAccuracy);

#endif