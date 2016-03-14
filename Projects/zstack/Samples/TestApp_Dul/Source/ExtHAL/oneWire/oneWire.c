/**
   *******************************************************************
   * @file		oneWire.c
   * @author	RQY
   * @version	V0.1
   * @date		2016-01-30
   * @brief		这个文件提供操作单总线传感器的函数
   *******************************************************************
   */

/* Includes ---------------------------------------------------------*/
#include "oneWire.h"

unsigned char	PORT;
unsigned char PIN;
unsigned char PX_Y;

/**
	* @brief	复位单总线器件
	* @param	pullLowTime: 复位单总线器件所需要的低电平脉冲的持续时间
	* @param	pullHighTime: 释放单总线所持续的时间
	* @return	无
	*/
void oneWireReset(unsigned int pullLowTime, unsigned int pullHighTime)
{
	outputMode();
	outputHigh();
	delay_us(50);
	outputLow();							// pull the 1-wire bus low
	delay_us(pullLowTime);		// pull the 1-wire bus low for reset pulse,  at least 480us
	outputHigh();							// pull the 1-wire bus high for releases the bus
	delay_us(pullHighTime);		// wait-out remaining initialisation window.
}

/**
	* @brief	检测单总线上是否有器件存在
	* @param	waitRspTime: 等待单总线器件发出响应信号的时间
	* @param	isSlavePullHigh: 单总线发出响应信号后，总线是否被单总线器件拉高；
			此参数可为一下两个值：
	*			@arg owTRUE: 	单总线被器件主动拉高
	*			@arg owFLASE:	单总线被上拉电阻拉高
	*	@param	isChangeToInputMode: 是否改变单总线输入模式；此参数可为以下值：
				@arg owTRUE: 	改变为输入模式
				@arg owFLASE:	不改变为输入模式
	* @return	unsigned char: 若单总线上存在器件，则返回owSuccess；否则，返回owFailure
	*/
unsigned char oneWireCheck(unsigned int waitRspTime, unsigned char isSlavePullHigh,
													 unsigned char isChangeToInputMode)
{
	unsigned char retry = 0;
	
	delay_us(waitRspTime);
	if ( isChangeToInputMode )
		inputMode();
	// Slave response ouput low level
	while ( readPin() && retry < 100)
	{
		retry++;
		delay_us(1);
	}
	if (retry >= 100)
		return owFailure;
	
	if ( isSlavePullHigh )
	{
		retry = 0;
		while (!readPin() && retry < 100)
		{
			retry++;
			delay_us(1);
		}
		if (retry >= 100)
			return owFailure;
	}
	else
	{
		outputHigh();		// releases the bus.
		delay_us(100);
	}
	
	outputMode();
	return owSuccess;
}

/**
	* @brief	写一字节到单总线器件
	* @param	data: 写到单总线器件中的数据
	* @return	无
	*/
void oneWireWrite(unsigned char data)
{
	int count;
	
	for (count = 0; count < 8; count++)
	{
		outputLow();
		delay_us(2);				// pull the 1-wire bus low to initiate write time-slot.
		if ( data & 0x01 )
			outputHigh();			// releases the bus for write 1 to the Slave.
		delay_us(30);				// wait until end of write slot.
		outputHigh();				// set the 1-wire bus high again for releases the bus.		
		delay_us(2);				// for more than 1us minimum.
		data >>= 1;
	}
	delay_us(5);
}

/**
	* @brief	从单总线器件中读一字节
	* @param	isLSB: 读出的数据是否低位在先，
			这个参数值可以是owTRUE，或者owFLASE
	* @param	isSlaveOutputReadTimeSlot: 单总线器件是否主动发出读时序，
			这个参数值可以是owTRUE，或者owFLASE
	* @return	unsigned char: 从单总线器件中读出的数据
	*/
unsigned char oneWireRead(unsigned char isLSB, unsigned char isSlaveOutputReadTimeSlot)
{
	unsigned char count, retry = 0;
	unsigned char data = 0;
	
	for (count = 0; count < 8; count++)
	{		
		if (isSlaveOutputReadTimeSlot)
		{
			while (readPin() && retry < 100)	// wait the slave output read time-slot
			{
				retry++;
				delay_us(1);
			}
			retry = 0;
			
			while (!readPin() && retry < 100)
			{
				retry++;
				delay_us(1);
			}
			delay_us(30);	
		}
		else
		{
			outputLow();
			delay_us(2);			// pull the 1-wire bus low to initiate read time-slot.			
			outputHigh();			// now let the 1-wire bus high for sample data of the Slave.
			delay_us(8);			// let device state stabilise.
		}
		
		if ( isLSB )
			data >>= 1;			// LSB first.
		else 
			data <<= 1;			// MSB first
		
		if ( readPin() == 1 )
			data |= 0x80;
		delay_us(60);			// wait until end of read slot.
		//data >>= 1;			// error		
	}
	
	return data;
}



void configOneWire(unsigned char port, unsigned char pin, unsigned char px_y)
{
	PORT = port;
	PIN = pin;
	PX_Y = px_y;
}

/**
	* @brief	微妙延时
	* @param	us: 需要延时的时间
	* @return	无
	*/
void delay_us(unsigned int us)
{
	while ( us-- )
	{
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
	}
}

/**
	* @brief	微妙延时
	* @param	ms: 需要延时的时间
	* @return	无
	*/
void delay_ms(unsigned int ms)
{
	while ( ms-- )
	{
		delay_us(1000);
	}
}







