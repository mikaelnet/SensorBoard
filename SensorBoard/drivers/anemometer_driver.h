/*
 * wind_driver.h
 *
 * Created: 2014-01-09 16:39:21
 *  Author: mikael.hogberg
 */ 


#ifndef ANEMOMETER_DRIVER_H_
#define ANEMOMETER_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

extern void anemometer_init();
extern uint16_t anemometer_counter ();

#endif /* ANEMOMETER_DRIVER_H_ */
