/*
 * board.cpp
 *
 * Created: 2013-12-30 21:01:36
 *  Author: mikael
 */ 

#include "board.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

volatile bool button_pressed = false;
ISR(PORTC_INT0_vect)
{
    button_pressed = true;
}

void board_init()
{
    // initialize LEDs
    LEDPORT.DIRSET = GLED_bm | RLED_bm;
    LEDPORT.OUTSET = GLED_bm | RLED_bm;
    
    // initialize button
    PORTC.INTCTRL = PORT_INT0LVL_MED_gc;
    PORTC.INT0MASK = _BV(2);
    PORTC.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

    // Enable medium level interrupts in the PMIC.
    PMIC.CTRL |= PMIC_MEDLVLEN_bm;
}

inline bool button_is_pressed() 
{
    return button_pressed;
}

inline void button_reset()
{
    button_pressed = false;
}

void thsen_enable()
{
	PORTD.DIRSET = _BV(3);
	PORTD.OUTSET = _BV(3);
}

void thsen_disable()
{
	PORTD.DIRCLR = _BV(3);
}

void ven_enable()
{
	PORTB.DIRSET = _BV(1);
	PORTB.OUTSET = _BV(1);
}

void ven_disable()
{
	PORTB.DIRCLR = _BV(1);
}

void sen_enable()
{
	PORTD.DIRSET = _BV(0);
	PORTD.OUTSET = _BV(0);
}

void sen_disable()
{
	PORTD.DIRCLR = _BV(0);
}

void rfen_enable()
{
	PORTB.DIRSET = _BV(3);
	PORTB.OUTSET = _BV(3);
}

void rfen_disable()
{
	PORTB.DIRCLR = _BV(3);
}

void vrefen_enable()
{
	PORTB.DIRSET = _BV(0);
	PORTB.OUTSET = _BV(0);
}

void vrefen_disable()
{
	PORTB.DIRCLR = _BV(0);
}
