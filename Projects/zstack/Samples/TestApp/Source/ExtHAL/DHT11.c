#include "dht11.h"

/* ��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ��� 
** ����1:������ 
** ����0:����
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
	DHT11_IO = 0; 					// ����DQ
	dht_delay_ms(30);    			// ��������18ms 
	DHT11_IO = 1; 				// ����DQ  
	dht_delay_us(30);     		// ��������20~40us 	
}

/* �ȴ�DHT11�Ļ�Ӧ 
** ����1:δ��⵽DHT11�Ĵ��� 
** ����0:����
*/
unsigned char DHT11Check(void)
{
	unsigned char retry=0;
	
	DHT11_IO_OUT;			// input mode
	
	while ( DHT11_IO && retry < 100 )		// DHT11������40~80us 
	{ 
		retry++; 
		dht_delay_us(1); 
	};
	
	if(retry >= 100)
		return 1; 
	else
		retry=0; 
	
	while ( !DHT11_IO && retry < 100 )		// DHT11���ͺ���ٴ�����40~80us 
	{ 
		retry++; 
		dht_delay_us(1); 
	};
	
	if(retry >= 100)
		return 1;	     
	else
		return 0; 
}

/* ��DHT11��ȡһ��λ 
** ����ֵ��1/0
*/
unsigned char DHT11ReadBit(void) 	  
{ 
	unsigned char retry = 0;
	
	while( DHT11_IO && retry < 100 )	//�ȴ���Ϊ�͵�ƽ��ÿһbit������ 50us �͵�ƽ��ʼ 
	{ 
		retry++;
		dht_delay_us(2);
	}
	retry=0;
	
	while( !DHT11_IO && retry < 100 )		//�ȴ���ߵ�ƽ 
	{ 
		retry++;
		dht_delay_us(2);
	}
	//while ( !DHT11_IO ); //�ȴ���ߵ�ƽ
	dht_delay_us(30);	//�ȴ�30us���ߵ�ƽ���� 26~28us ʱ������Ϊ 0������ 70us Ϊ 1
	
	if( DHT11_IO )
		return 1; 
	else 
		return 0;
}


/* ��DHT11��ȡһ���ֽ� 
** ����ֵ������������ 
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


/* ��DHT11��ȡһ������ 
** temp:�¶�ֵ(��Χ:0~50��) 
** humi:ʪ��ֵ(��Χ:20%~90%) 
** ����ֵ��0,����; 1,��ȡʧ��
*/
unsigned char DHT11ReadData(unsigned char* temp, unsigned char* humi)     
{         
 	unsigned char buf[5]; 
	unsigned char i;
	buf[0] = buf[1] = buf[2] = buf[3] =buf[4] = 0;
	
	DHT11Reset();
	
	if ( !DHT11Check() ) 
	{ 
		for ( i = 0; i < 5; i++ )		//��ȡ40λ���� 
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

// ΢����ʱ
void dht_delay_us(unsigned int us)
{
	while ( us-- )
	{
		asm("NOP");
		asm("NOP");
		asm("NOP");
	}
}

// ������ʱ
void dht_delay_ms(unsigned int ms)
{
	while ( ms-- )
	{
		dht_delay_us(1000);
	}
}

