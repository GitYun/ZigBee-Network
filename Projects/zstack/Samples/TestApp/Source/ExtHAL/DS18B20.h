#ifndef __DS18B20_H
#define __DS18B20_H

#include "ioCC2530.h"

/*----------------- One-Wire Bus Define --------------------*/

#define PERIPH_PORT					0
#define DS_PIN							7
#define DS_PORT_PIN					P0_7
#define PxSEL								P0SEL
#define PxDIR								P0DIR
#define PxINP								P0INP

/****************** x means PORT Number *********************/
#define afio()							( PxSEL |= (1 << DS_PIN) )
#define gpio()							( PxSEL &= ~(1 << DS_PIN) )
#define outputMode()				( PxDIR |= (1 << DS_PIN) )
#define inputMode()					( PxDIR &= ~(1 << DS_PIN) )
#define outputLow()					( DS_PORT_PIN = 0 )
#define outputHigh()				( DS_PORT_PIN = 1 )
#define pullUp()						P2INP &= ~(0x20 << PERIPH_PORT); \
															PxINP &= ~(1 << DS_PIN)
#define pullDown()					P2INP |= (0x20 << PERIPH_PORT); \
															PxINP &= ~(1 << DS_PIN)
#define readPin()						( DS_PORT_PIN )


/*--------------- DS18B20 Function Commands -----------------*/

#define	CONVERT_T		0x44		// Initiates temperature conversion.

#define READ_REG		0xBE		// Reads the entire scratchpad including the CRC byte.

#define WRITE_REG		0x4E		// Writes data into scratchpad bytes 2,3, 
													// and 4(TH, TL, and configuration registers).

#define COPY_REG		0x48		// Copies TH, TL, and configuration register data
													// from the scratchpad to EEPROM.

#define RECALL_E2		0xB8		// Recalls TH, TL, and configuration register data
													// from EEPROM to the scratchpad.

#define READ_POWER	0xB4		// Signals DS18B20 power supply mode to the master.


/*---------------- DS18B20 ROM Commands --------------------*/

#define SEARCH_ROM	0xF0		// The master identify the ROM codes of all slave devices on the bus.

#define READ_ROM		0x33		// The master to read the slave's 64-bit ROM
													// code without using the Search ROM procedure.
													// This commmand can only be used when there is one slave on the bus.

#define MATCH_ROM		0x55		// The match ROM commmand followed by a 64-bit
													// ROM code sequence allows the bus master to
													// address a specific slave device on a multidrop or single-drop bus.

#define SKIP_ROM		0xCC		// The master can use this command to address all
													// devices on the bus simultaneously without sending 
													// out any ROM code information.

#define ALARM_SEARCH 0xEC

/*------------------ DS18B20 Functions ---------------------*/
unsigned char dsInit(void);
float dsReadT(void);

void ds_delay_us(unsigned int us);
void ds_delay_ms(unsigned int ms);


/*------------------ One Wire Bus Functions ----------------*/

void oneWireReset(void);
unsigned char oneWireCheck(void);
void oneWireWrite(int data);
unsigned char oneWireRead(void);

#endif