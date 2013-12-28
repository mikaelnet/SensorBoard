/*
 * ds1820_tests.cpp
 *
 * Created: 2013-12-28 13:59:53
 *  Author: mikael
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include <util/atomic.h>

#include "../drivers/ds1820_driver.h"

DS1820 ds1820(&PORTD, 5);

#ifdef __cplusplus
extern "C" {
#endif

void ds1820_tests()
{
	if (!ds1820.startConversion())
		puts_P(PSTR("No power on pin"));
	_delay_ms(750);	// 12bit=750ms conversion time
	ds1820.readFirst();
}

#ifdef __cplusplus
}
#endif
