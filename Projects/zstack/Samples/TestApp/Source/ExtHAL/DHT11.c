#include "dht11.h"

/* 初始化DHT11的IO口 DQ 同时检测DHT11的存在 
** 返回1:不存在 
** 返回0:存在
*/
unsigned char DHT11IOInit(void)
{	
	P0SEL &= ~0x40;		// Px_y used as GPIO function
	DHT11_IO_IN;			// Px_y input mode
	P0INP &= ~0x40;		// pull up or pull down IO pin
	DHT11_IQ_PULL_UP;	// pull up
	
	DHT11Reset();
	return DHT11Check();
}

void DHT11Reset(void)
{
	DHT11_IO_OUT;					// ouptput mode
	DHT11_IO = 0; 					// 拉低DQ
	dht_delay_ms(30);    			// 拉低至少18ms 
	DHT11_IO = 1; 				// 拉高DQ  
	dht_delay_us(30);     		// 主机拉高20~40us 	
}

/* 等待DHT11的回应 
** 返回1:未检测到DHT11的存在 
** 返回0:存在
*/
unsigned char DHT11Check(void)
{
	unsigned char retry=0;
	
	DHT11_IO_OUT;			// input mode
	
	while ( DHT11_IO && retry < 100 )		// DHT11会拉低40~80us 
	{ 
		retry++; 
		dht_delay_us(1); 
	};
	
	if(retry >= 100)
		return 1; 
	else
		retry=0; 
	
	while ( !DHT11_IO && retry < 100 )		// DHT11拉低后会再次拉高40~80us 
	{ 
		retry++; 
		dht_delay_us(1); 
	};
	
	if(retry >= 100)
		return 1;	     
	else
		return 0; 
}

/* 从DHT11读取一个位 
** 返回值：1/0
*/
unsigned char DHT11ReadBit(void) 	  
{ 
	unsigned char retry = 0;
	
	while( DHT11_IO && retry < 100 )	//等待变为低电平，每一bit数据以 50us 低电平开始 
	{ 
		retry++;
		dht_delay_us(2);
	}
	retry=0;
	
	while( !DHT11_IO && retry < 100 )		//等待变高电平 
	{ 
		retry++;
		dht_delay_us(2);
	}
	//while ( !DHT11_IO ); //等待变高电平
	dht_delay_us(30);	//等待30us，高电平持续 26~28us 时，数据为 0，持续 70us 为 1
	
	if( DHT11_IO )
		return 1; 
	else 
		return 0;
}


/* 从DHT11读取一个字节 
** 返回值：读到的数据 
*/
unsigned char DHT11ReadByte(void)     
{         
  unsigned char i;
	volatile unsigned char dat = 0;
	 
	for ( i = 0; i < 8; i++ )  
	{ 
   	dat <<= 1;
    dat |= DHT11ReadBit(); 
  }
	
  return dat; 
} 


/* 从DHT11读取一次数据 
** temp:温度值(范围:0~50°) 
** humi:湿度值(范围:20%~90%) 
** 返回值：0,正常; 1,读取失败
*/
unsigned char DHT11ReadData(unsigned char* temp, unsigned char* humi)     
{         
 	unsigned char buf[5]; 
	unsigned char i;
	buf[0] = buf[1] = buf[2] = buf[3] =buf[4] = 0;
	
	DHT11Reset();
	
	if ( !DHT11Check() ) 
	{ 
		for ( i = 0; i < 5; i++ )		//读取40位数据 
		{ 
			buf[i] = DHT11ReadByte(); 
		}
		
		if (( buf[0] + buf[1] + buf[2] + buf[3] ) == buf[4]) 
		{ 
			*humi = buf[0]; 
			*temp = buf[2];
			
			return 0;
		} 
	}
	
	return 1;	     
}

// 微妙延时
void dht_delay_us(unsigned int us)
{
	while ( us-- )
	{
		asm("NOP");
		asm("NOP");
		asm("NOP");
	}
}

// 毫秒延时
void dht_delay_ms(unsigned int ms)
{
	while ( ms-- )
	{
		dht_delay_us(1000);
	}
}

