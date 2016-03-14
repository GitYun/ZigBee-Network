/**
   *******************************************************************
   * @file		dht11.h
   * @author	RQY
   * @version	V1.0
   * @date		2016-01-30
   * @brief		����ļ���������DHT11�����к���ԭ��
   *******************************************************************
   */

/* Define to prevent recursive inclusion ----------------------------*/
#ifndef __DHT11_H
#define __DHT11_H

/* Includes ---------------------------------------------------------*/
#include "oneWire.h"

/* DHT11 Pin Define -------------------------------------------------*/
#define	PORT	0
#define PIN		6
#define PX_Y	P0_6

/* DHT11 Functions Prototype-----------------------------------------*/
unsigned char dht11Init(void);
unsigned char dht11ReadData(unsigned char* tmep, unsigned char* humi);

#endif