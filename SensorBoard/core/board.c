/*
 * board.cpp
 *
 * Created: 2013-12-30 21:01:36
 *  Author: mikael
 */ 

#include "board.h"

#include <avr/io.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void init_board()
{
	// Init leds
	LEDPORT.DIRSET = GLED_bm | RLED_bm;
	LEDPORT.OUTSET = GLED_bm | RLED_bm;
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


#ifdef __cplusplus
}
#endif
