#include "ds18b20.h"


unsigned char dsInit()
{
	gpio();						// P0_6 used as GPIO function
	outputMode();			// output mode
	pullUp();					// pull-up
	
	oneWireReset();
	return oneWireCheck();
}

float dsReadT()
{
	unsigned char temp1, temp2;
	unsigned int temp3;
	signed int temp4;
	float result;
	static unsigned char initFalg = 1;
	
	if ( initFalg )
	{
		if ( dsInit() == 0 )
		{		
			initFalg = 0;
		}
		else
		{
			return ( -10000.0 );
		}
	}	
	
	
	oneWireReset();
	if (oneWireCheck() == 1)
		return ( -10000.0 );
	oneWireWrite(SKIP_ROM);
	oneWireWrite(CONVERT_T);
	
/* The master can issue read time slots after the Convert T command and
** the DS18B20 will respond by transmitting a 0 while the temperature 
** conversion is in progress and a 1 when the conversion is done.
*/	
	while (oneWireRead() == 0);	//check the DS18B20 if busy?
	
	oneWireReset();
	if (oneWireCheck() == 1)
		return ( -10000.0 );
	oneWireWrite(SKIP_ROM);
	oneWireWrite(READ_REG);
	
	temp1 = oneWireRead();		// Temperature LSB(50h)
	temp2 = oneWireRead();		// Temperature MSB(05h)
	temp3 = (unsigned int)( (temp2 << 8) | temp1);
	
	if (temp2 & 0xF8)		// 温度数据是否有符号位，若有则为负温度
		temp4 = ((temp3^0x7FFF) + 0x0001);	// 为负温度时，数据为补码，所以转换为负数的原码
	else
		temp4 = (signed int)temp3;
	
	result = (float)(temp4 * 0.0625);	// Calculation for DS18S20 with 0.0625 deg C resolution
	//result = (float)temp4 / 16.0;
	//result = (float)temp4 / 2.0;		// Calculation for DS18S20 with 0.5 deg C resolution
	return result;
}


void oneWireReset()
{
	outputMode();
	outputHigh();
	ds_delay_us(50);
	outputLow();			// pull the 1-wire bus low
	ds_delay_us(500);		// pull the 1-wire bus low for reset pulse,  at least 480us
	outputHigh();			// pull the 1-wire bus high for releases the bus
//	delay_us(500);		// wait-out remaining initialisation window.
}

/*
** return		0 - if the salve presence, or 1
*/
unsigned char oneWireCheck()
{
	unsigned char isSlave;
	ds_delay_us(30);
	isSlave = (unsigned char)readPin();
	outputHigh();		// releases the bus.
	ds_delay_us(100);
	return isSlave;
}

void oneWireWrite(int data)
{
	int count;
	
	for (count = 0; count < 8; count++)
	{
		outputLow();
		ds_delay_us(2);			// pull the 1-wire bus low to initiate write time-slot.
		if ( data & 0x01 )
			outputHigh();	// releases the bus for write 1 to the Slave.
		ds_delay_us(30);			// wait until end of write slot.
		outputHigh();		// set the 1-wire bus high again for releases the bus.		
		ds_delay_us(2);			// for more than 1us minimum.
		data >>= 1;
	}
	ds_delay_us(5);
}


unsigned char oneWireRead()
{
	int count;
	unsigned char data = 0;
	
	for (count = 0; count < 8; count++)
	{		
		outputLow();
		ds_delay_us(2);			// pull the 1-wire bus low to initiate read time-slot.
		outputHigh();			// now let the 1-wire bus high for sample data of the Slave.
		ds_delay_us(8);			// let device state stabilise.
		data >>= 1;				// LSB first.
		if ( readPin() == 1 )
			data |= 0x80;
		ds_delay_us(60);			// wait until end of read slot.
		//data >>= 1;			// error		
	}
	
	return data;
}



// 微妙延时
void ds_delay_us(unsigned int us)
{
	while ( us-- )
	{
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
	}
}

// 毫秒延时
void ds_delay_ms(unsigned int ms)
{
	while ( ms-- )
	{
		ds_delay_us(1000);
	}
}

