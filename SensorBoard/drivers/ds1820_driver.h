/*
 * ds1820_driver.h
 *
 * Created: 2012-05-20 13:05:45
 *  Author: mikael
 */ 


#ifndef DS1820_DRIVER_H_
#define DS1820_DRIVER_H_

#if DS1820_ENABLE==1

#include <avr/io.h>
#include <stdbool.h>

extern void DS1820_begin (PORT_t *port, uint8_t pin);
extern bool DS1820_startConversion ();

extern uint16_t DS1820_readTemperature (uint8_t *id);
extern uint16_t DS1820_readFirst ();	// replace this one

extern float DS1820_convert2temperature (uint16_t reading);

// Requires an allocated buf of at least 24 bytes
extern void DS1820_addressToString (uint8_t *id, char *buf);

#endif /* DS1820_ENABLE==1 */

#endif /* DS1820_DRIVER_H_ */