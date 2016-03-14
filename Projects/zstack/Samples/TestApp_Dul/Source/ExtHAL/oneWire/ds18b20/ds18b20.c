/**
   *******************************************************************
   * @file		ds18b20.c
   * @author	RQY
   * @version	V1.0
   * @date		2016-01-30
   * @brief		这个文件包含操作ds18b20的所有函数实现
   *******************************************************************
   */

/* Define to prevent recursive inclusion ----------------------------*/
#include "ds18b20.h"

/** DS18B20 Gobal Variable */
unsigned char preAccuracyVlaue = DEFAULT;

/**
	* @brief	初始化DS18B20
	* @param	无 
	* @return	unsigned char: 若单总线上存在DS18B20，则返回owSucess；否则，返回owFailure
	*/
unsigned char dsInit(void)
{
	gpio();						// P0_6 used as GPIO function
	outputMode();			// output mode
	pullUp();					// pull-up
	
	configOneWire(0, 7, P0_7);
	
	oneWireReset(500, 30);
	return oneWireCheck(1, owFLASE);
}

/**
	* @brief	读取DS18B20的温度数据
	* @param	outputAccuracy: 控制温度输出的精度，此参数可以为一下值：
	*		@arg HALF: 输出精度为0.5摄氏度
	*		@arg QUARTER: 输出精度为0.25摄氏度
	*		@arg EIGHTH: 输出精度为0.125摄氏度
	*		@arg SIXTEENTH: 输出精度为0.0625摄氏度
	* @return	float: DS18B20的温度数据 
	*/
float dsReadT(unsigned char outputAccuracy)
{
	unsigned char TH, TL, Config;
	unsigned char temp1, temp2;
	unsigned int temp3;
	signed int temp4;
	float result;
	
	if ( (preAccuracyVlaue != outputAccuracy) && outputAccuracy < 4 )
	{
		do
		{
			oneWireReset(500, 30);
			if ( oneWireCheck(1, owFLASE) == owFailure )
				return ( -10000.0 );
			oneWireWrite( READ_REG );
			(void)oneWireRead( owTRUE, owFLASE);	// first LSB, 1-wire bus master output read time-slot
			(void)oneWireRead( owTRUE, owFLASE );
			TH = oneWireRead( owTRUE, owFLASE );
			TL = oneWireRead( owTRUE, owFLASE );
			Config = oneWireRead( owTRUE, owFLASE );
			
			if ( (Config & 0x60) != outputAccuracy )
			{
				oneWireReset(500, 30);
				if (oneWireCheck(1, owFLASE) == owFailure )
					return ( -10000.0 );
				oneWireWrite( WRITE_REG );
				oneWireWrite( TH );
				oneWireWrite( TL );
				oneWireWrite( outputAccuracy );
			}
			else
			{
				preAccuracyVlaue = outputAccuracy;	// 保存DS18B20的上一次输出精度配置值
			}
		} while ( preAccuracyVlaue != outputAccuracy );
	}
	
	oneWireReset(500, 30);
	if ( oneWireCheck(1, owFLASE) == owFailure )
		return ( -10000.0 );
	oneWireWrite(SKIP_ROM);
	oneWireWrite(CONVERT_T);
	
/*
 * The master can issue read time slots after the Convert T command and
 * the DS18B20 will respond by transmitting a 0 while the temperature 
 * conversion is in progress and a 1 when the conversion is done.
 */	
	while (oneWireRead( owTRUE, owFLASE ) == 0);	// check the DS18B20 if busy?
	
	oneWireReset(500, 30);
	if ( oneWireCheck(1, owFLASE) == owFailure )
		return (-10000.0);
	oneWireWrite(SKIP_ROM);
	oneWireWrite(READ_REG);
	temp1 = oneWireRead( owTRUE, owFLASE );		// Temperature LSB(50h)
	temp2 = oneWireRead( owTRUE, owFLASE );		// Temperature MSB(05h)
	temp3 = (unsigned int)( (temp2 << 8) | temp1);
	
	if (temp2 & 0xF0)		// Temperature has a sign bits, it is negtive temperature.
		// When it is negtive, data is binary complement, so covert it to binary source code.
		temp4 = ((temp3^0x7FFF) + 0x0001);
	else
		temp4 = (signed int)temp3;
	
	switch ( outputAccuracy )
	{
		case HALF: 
			result = (float)(temp4 * 0.5);	// Calculation for DS18S20 with 0.5 deg C resolution
			//result = (float)temp3 / 2.0;
			break;
		case QUARTER:
			result = (float)(temp4 * 0.25);
			break;
		case EIGHTH:
			result = (float)(temp4 * 0.125);
			break;
		case SIXTEENTH:
			result = (float)(temp4 * 0.0625);	// Calculation for DS18B20 with 0.0625 deg C resolution.		
			//result = (float)temp3 / 16.0;
			break;
		default:
			result = -10000.0;
			break;
	}
		
	return result;
}
