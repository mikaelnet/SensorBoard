/*
 * ds1820_driver.h
 *
 * Created: 2012-05-20 13:05:45
 *  Author: mikael
 */ 


#ifndef DS1820_DRIVER_H_
#define DS1820_DRIVER_H_

#include <avr/io.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ds1820_termometer_init(PORT_t *port, uint8_t pin);
uint16_t ds1820_termometer_read ();
float ds1820_to_temperature (uint16_t temperature);

#ifdef __cplusplus
}
#endif

#endif /* DS1820_DRIVER_H_ */