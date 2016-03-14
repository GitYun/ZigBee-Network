/**
   ************************************************************************
   * @file		dht11.c	
   * @author	RQY
   * @version V1.0
   * @date		2016-01-30
   * @brief		这个文件提供操作DHT11的函数实现
   *************************************************************************
   */

/* Define to prevent recursive inclusion ---------------------------------*/
#include "dht11.h"

/**
  * @brief	初始化DHT11
  * @param	无
  * @return	unsigned char: 若初始化DHT11成功，则返回owSuccess；否则，返回owFailure
  */
unsigned char dht11Init(void)
{
	gpio();
	inputMode();
	pullUp();
	
	configOneWire(0, 6, P0_6);
	
	oneWireReset(20, 30);
	return oneWireCheck(1, owTRUE, owTRUE);
}

/**
  * @brief	读取dht11的数据
  * @param	temp: dht11的温度数据
	* @param	humi: dht11的湿度数据
  * @return	unsigned char: 若读取数据成功，返回owSuccess; 否则，返回owFailure
  */
unsigned char dht11ReadData(unsigned char *temp, unsigned char *humi)     
{         
 	unsigned char buf[5] = {0}; 
	unsigned char i;
	
	oneWireReset(20, 30);
	
	if ( oneWireCheck(1, owTRUE) == owSuccess ) 
	{ 
		for ( i = 0; i < 5; i++ )		//read 40-bits data 
		{ 
			buf[i] = oneWireRead( owFLASE, owTRUE );// first MSB, 1-wire slave ouput read time-slot
		}
		
		if (( buf[0] + buf[1] + buf[2] + buf[3] ) == buf[4]) 
		{ 
			*humi = buf[0]; 
			*temp = buf[2]; 
		}
	}
	else 
		return owFailure;
	
	return owSuccess;	     
}