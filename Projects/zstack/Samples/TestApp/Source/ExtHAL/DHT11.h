#ifndef __DHT11_H
#define __DHT11_H

#include "iOCC2530.h"

#define DHT11_IO						P0_6
#define DHT11_IO_OUT				( P0DIR |= 0x40 )			// output mode
#define DHT11_IO_IN					( P0DIR &= ~0x40 )		// input mode
// #define DHT11_IQ_PULL_DOWN	( P2INP |= 0x20 )		// pull-down
#define DHT11_IQ_PULL_UP		( P2INP &= ~0x20 )		// pull-up IO Port


//#define PERIPH_PORT					0
//#define DHT_PIN							6
//#define DHT_PORT_PIN				P0_6
//#define PxSEL								P0SEL
//#define PxDIR								P0DIR
//#define PxINP								P0INP

/****************** x means PORT Number *********************/
//#define afio()							( PxSEL |= (1 << DHT_PIN) )
//#define gpio()							( PxSEL &= ~(1 << DHT_PIN) )
//#define outputMode()				( PxDIR |= (1 << DHT_PIN) )
//#define inputMode()					( PxDIR &= ~(1 << DHT_PIN) )
//#define outputLow()					( DHT_PORT_PIN = 0 )
//#define outputHigh()				( DHT_PORT_PIN = 1 )
//#define pullUp()						P2INP &= ~(0x20 << PERIPH_PORT); \
//															PxINP &= ~(1 << DHT_PIN)
//#define pullDown()					P2INP |= (0x20 << PERIPH_PORT); \
//															PxINP &= ~(1 << DHT_PIN)
//#define readPin()						( DHT_PORT_PIN )


// ��ʼ�� DHT11
unsigned char DHT11IOInit(void);

// ��ȡ��ʪ��
unsigned char DHT11ReadData(unsigned char* tmep, unsigned char* humi);

// ��ȡһ���ֽ�
unsigned char DHT11ReadByte(void);

// ��ȡһ��λ
unsigned char DHT11ReadBit(void);

// ����Ƿ���� DHT11
unsigned char DHT11Check(void);

// ��λ DTH11
void DHT11Reset(void);

// ΢����ʱ
void dht_delay_us(unsigned int us);

// ������ʱ
void dht_delay_ms(unsigned int ms);

#endif
