/*
 * mcp79410_driver.cpp
 *
 * Created: 2013-12-30 21:35:55
 *  Author: mikael
 */ 

#if MCP79410_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "mcp79410_driver.h"

static volatile bool _minuteInterrupt = false;
// RAIN
ISR(PORTD_INT0_vect)
{
	_minuteInterrupt = true;
}

void MCP79410_begin ()
{
	// Enable edge interrupt on 
	PORTD.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTD.INT0MASK = _BV(2);
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

}


#endif