/**
   *******************************************************************
   * @file		oneWire.c
   * @author	RQY
   * @version	V0.1
   * @date		2016-01-30
   * @brief		����ļ��ṩ���������ߴ������ĺ���
   *******************************************************************
   */

/* Includes ---------------------------------------------------------*/
#include "oneWire.h"

unsigned char	PORT;
unsigned char PIN;
unsigned char PX_Y;

/**
	* @brief	��λ����������
	* @param	pullLowTime: ��λ��������������Ҫ�ĵ͵�ƽ����ĳ���ʱ��
	* @param	pullHighTime: �ͷŵ�������������ʱ��
	* @return	��
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
	* @brief	��ⵥ�������Ƿ�����������
	* @param	waitRspTime: �ȴ�����������������Ӧ�źŵ�ʱ��
	* @param	isSlavePullHigh: �����߷�����Ӧ�źź������Ƿ񱻵������������ߣ�
			�˲�����Ϊһ������ֵ��
	*			@arg owTRUE: 	�����߱�������������
	*			@arg owFLASE:	�����߱�������������
	*	@param	isChangeToInputMode: �Ƿ�ı䵥��������ģʽ���˲�����Ϊ����ֵ��
				@arg owTRUE: 	�ı�Ϊ����ģʽ
				@arg owFLASE:	���ı�Ϊ����ģʽ
	* @return	unsigned char: ���������ϴ����������򷵻�owSuccess�����򣬷���owFailure
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
	* @brief	дһ�ֽڵ�����������
	* @param	data: д�������������е�����
	* @return	��
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
	* @brief	�ӵ����������ж�һ�ֽ�
	* @param	isLSB: �����������Ƿ��λ���ȣ�
			�������ֵ������owTRUE������owFLASE
	* @param	isSlaveOutputReadTimeSlot: �����������Ƿ�����������ʱ��
			�������ֵ������owTRUE������owFLASE
	* @return	unsigned char: �ӵ����������ж���������
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
	* @brief	΢����ʱ
	* @param	us: ��Ҫ��ʱ��ʱ��
	* @return	��
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
	* @brief	΢����ʱ
	* @param	ms: ��Ҫ��ʱ��ʱ��
	* @return	��
	*/
void delay_ms(unsigned int ms)
{
	while ( ms-- )
	{
		delay_us(1000);
	}
}







