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

typedef struct DHT22_struct {
	PORT_t *port;
	uint8_t pin_bm;
	// Report the humidity in .1 percent increments, such that 635 means 63.5% relative humidity
	int16_t lastHumidity;	
	// Get the temperature in decidegrees C, such that 326 means 32.6 degrees C.
	// The temperature may be negative, so be careful when handling the fractional part.
	int16_t lastTemperature;
} DHT22_t;

extern void DHT22_init(DHT22_t *dht22, PORT_t *port, uint8_t pin);
extern DHT22_ERROR_t DHT22_readData(DHT22_t *dht22);


extern short int DHT22_getHumidityInt(DHT22_t *dht22);	// Hum*10
extern short int DHT22_getTemperatureCInt(DHT22_t *dht22);	// Temp*10
extern double DHT22_dewPoint(double celsius, double humidity);

#endif

#endif /* DHT22_DRIVER_H_ */