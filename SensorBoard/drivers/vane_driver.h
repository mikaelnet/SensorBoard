/*
 * vane_driver.h
 *
 * Created: 2013-12-28 10:56:40
 *  Author: mikael
 */ 


#ifndef VANE_DRIVER_H_
#define VANE_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct WindDirection_struct {
    uint8_t index;
    uint16_t adcValue;
    uint16_t adcThreshold;
    float sinCoefficient;
    float cosCoefficient ;
} WindDirection_t;

typedef struct Vane_struct {
    float direction;
    float force;
} Vane_t;

extern void vane_init ();
extern int8_t vane_parseReading (uint16_t reading, int16_t *diff); // Make static?
extern void vane_reset ();
extern void vane_add (uint8_t directionIndex, uint16_t windCounter);
extern Vane_t vane_calculate ();

#endif /* VANE_DRIVER_H_ */

