/*
 * rain_driver.c
 *
 * Created: 2014-01-09 16:49:33
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "raingauge_driver.h"

volatile uint16_t RainGaugeCounter;
// RAIN counter. One tick represents 0.2794mm rain
ISR(PORTB_INT0_vect)
{
	RainGaugeCounter ++;
}

void raingauge_init()
{
	RainGaugeCounter = 0;
	
	// Enable medium level interrupt INT0 on falling edge of pin PB2.
	PORTB.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTB.INT0MASK = _BV(2);
	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

	// Enable medium level interrupts in the PMIC.
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
}

uint16_t raingauge_counter () {
	uint16_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		count = RainGaugeCounter;
	}
	return count;
}

