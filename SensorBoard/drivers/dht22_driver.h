/*
 * dht22_driver.h
 *
 * Created: 2012-11-27 20:55:41
 *  Author: mikael
 */ 


#ifndef DHT22_DRIVER_H_
#define DHT22_DRIVER_H_

#if DHT22_ENABLE==1

#include <avr/io.h>

#define DHT22_ERROR_VALUE -995

typedef enum
{
	DHT_ERROR_NONE = 0,
	DHT_BUS_HUNG,
	DHT_ERROR_NOT_PRESENT,
	DHT_ERROR_ACK_TOO_LONG,
	DHT_ERROR_SYNC_TIMEOUT,
	DHT_ERROR_DATA_TIMEOUT,
	DHT_ERROR_CHECKSUM,
	DHT_ERROR_TOOQUICK
} DHT22_ERROR_t;

extern void DHT22_begin(PORT_t *port, uint8_t pin);
extern DHT22_ERROR_t DHT22_readData();
extern short int DHT22_getHumidityInt();
extern short int DHT22_getTemperatureCInt();
extern void DHT22_clockReset();
#if !defined(DHT22_NO_FLOAT)
extern float DHT22_getHumidity();
extern float DHT22_getTemperatureC();
#endif


#endif

#endif /* DHT22_DRIVER_H_ */