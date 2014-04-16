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

#include "onewire_driver.h"

bool DS1820_FindFirst(OneWire_t *wire, uint8_t *id);
bool DS1820_FindNext(OneWire_t *wire, uint8_t *id);
void DS1820_StartConvertion(OneWire_t *wire, uint8_t *id);
uint16_t DS1820_ReadTemperature(OneWire_t *wire, uint8_t *id);

#endif /* DS1820_ENABLE==1 */

#endif /* DS1820_DRIVER_H_ */