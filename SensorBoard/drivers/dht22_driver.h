/*
 * dht22_driver.h
 *
 * Created: 2012-11-27 20:55:41
 *  Author: mikael
 */ 


#ifndef DHT22_DRIVER_H_
#define DHT22_DRIVER_H_

#include "hardware.h"
#include <avr/io.h>

#define MAXTIMINGS 85

#define DHT11	11
#define DHT22	22
#define DHT21	21
#define AM2301	DHT21
#define AM2302	DHT22

#ifndef NAN
#define NAN		0
#endif

#ifdef __cplusplus
extern "C" {
#endif

void dht_init(PORT_t *port, uint8_t pin, uint8_t type);
float dht_readTemperature();


uint16_t dht_readHumidity();

#ifdef __cplusplus 
};
#endif

#endif /* DHT22_DRIVER_H_ */