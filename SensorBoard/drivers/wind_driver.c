/*
 * wind_driver.c
 *
 * Created: 2014-01-09 16:37:56
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "wind_driver.h"

volatile uint16_t WindCount;
// WIND counter. One tick per second means 2.4 km/h
ISR(PORTA_INT0_vect)
{
	WindCount ++;
	// Potential, test PORTA.INTFLAGS & 0x01;
}

void wind_init() 
{
	WindCount = 0;
	
	// Enable medium level interrupt INT0 on falling edge of pin PA2.
	PORTA.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTA.INT0MASK = _BV(2);
	PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

	// Enable medium level interrupts in the PMIC.
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
}

// This method should be called periodically. 
// Avg wind is measured during a 10 minute period.
// Gusts wind is measured during a 2 minute period.
uint16_t wind_counter () {
	uint16_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		count = WindCount;
	}
	return count;
}

