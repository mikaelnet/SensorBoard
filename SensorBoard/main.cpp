/*
 * SensorBoard.cpp
 *
 * Created: 2013-12-27 22:14:11
 *  Author: mikael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include "core/cpu.h"
#include "core/console.h"


#include "tests/dht22_tests.h"
#include "tests/ds1820_tests.h"

void setup_console()
{
	console_init();
}	

#define LEDPORT	PORTC
#define GLED_bm	_BV(0)
#define RLED_bm	_BV(1)

#define gled_on()	(LEDPORT.OUTCLR = GLED_bm)
#define gled_off()	(LEDPORT.OUTSET = GLED_bm)
#define rled_on()	(LEDPORT.OUTCLR = RLED_bm)
#define rled_off()	(LEDPORT.OUTSET = RLED_bm)

void thsen_enable()
{
	puts_P(PSTR("TH enable"));
	gled_on();
	PORTD.DIRSET = _BV(3);
	PORTD.OUTSET = _BV(3);
}

void thsen_disable()
{
	PORTD.DIRCLR = _BV(3);
	gled_off();
	puts_P(PSTR("TH disable"));
}

void init_leds ()
{
	LEDPORT.DIRSET = GLED_bm | RLED_bm;
	LEDPORT.OUTSET = GLED_bm | RLED_bm;
}

void setup()
{
#if F_CPU == 32000000UL
	cpu_set_32_MHz();
#endif
	init_leds();
	thsen_enable();
	setup_console();
}

void loop()
{
	puts_P(PSTR("Sensor board v0.1.1, " __TIMESTAMP__));
	printf_P(PSTR("Running at %d MHz\n"), F_CPU/1000000UL);

	//_delay_ms(100);	// wait for it to become stable
	
	//dht22_tests();
	ds1820_tests();

	//thsen_disable();

	_delay_ms(2000);
}

int main(void)
{
	setup();
	sei();
    while(1)
    {
        //TODO:: Please write your application code 
		loop();
    }
}