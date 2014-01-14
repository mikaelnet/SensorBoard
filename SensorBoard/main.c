/*
 * SensorBoard.cpp
 *
 * Created: 2013-12-27 22:14:11
 *  Author: mikael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>


#include "core/cpu.h"
#include "core/board.h"
#include "core/console.h"

#include "application/terminal.h"
#include "application/clock.h"
#include "application/thermometer.h"
#include "application/hygrometer.h"
#include "application/barometer.h"
#include "application/windrain.h"

#ifdef TEST_CODE
#include "tests/dht22_tests.h"
#include "tests/ds1820_tests.h"
#include "tests/bmp085_tests.h"
#include "tests/adc_tests.h"
#include "tests/mcp79410_tests.h"
#include "tests/sd_tests.h"

#include "drivers/vane_driver.h"
#endif

static void boot()
{
    cpu_init();
    board_init();
    console_init();
    sei();
    terminal_init();
    clock_init();
    thermometer_init();
    hygrometer_init();
    barometer_init();
    windrain_init();
    
#ifdef TEST_CODE
#if DHT22_ENABLE==1
	dht22_tests_setup();
#endif
#if DS1820_ENABLE==1
	ds1820_tests_setup();
#endif
#if BMP085_ENABLE==1
	bmp085_tests_setup();
#endif
#if MCP79410_ENABLE==1
	mcp79410_setup();
#endif
#endif

#ifdef TEST_CODE
	adc_tests_setup();
	irq_tests_setup();
    //vane_init();
    //sd_tests_setup();
#endif
}

#ifdef TEST_CODE
bool loop()
{
    bool canSleep = true;
#if DHT22_ENABLE==1
	canSleep &= dht22_tests();
#endif
#if DS1820_ENABLE==1
	canSleep &= ds1820_tests();
#endif
#if BMP085_ENABLE==1	
	canSleep &= bmp085_tests();
#endif
#if MCP79410_ENABLE==1
	canSleep &= mcp79410_tests();
#endif
	canSleep &= adc_tests();
	canSleep &= irq_tests();
    return canSleep;
}
#endif

int main(void)
{
    boot();
	//puts_P(PSTR("Sensor board v0.1.1, " __TIMESTAMP__));
    
	gled_on();  // Indicate that the board is active.
    
    while (1) {
        terminal_process();
        
        gled_off();
        cpu_try_sleep();
        gled_on();
    }
}

