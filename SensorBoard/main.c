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
#include "core/process.h"
#include "core/console.h"

#include "application/terminal.h"
#include "application/clock.h"
#include "application/thermometer.h"
#include "application/hygrometer.h"
#include "application/barometer.h"
#include "application/wind.h"
#include "application/rain.h"

static void boot()
{
    // Initialize core components
    cpu_init();
    board_init();
    console_init();
    sei();
    
    // Initialize all applications
    terminal_init();
    clock_init();
    thermometer_init();
    hygrometer_init();
    barometer_init();
    wind_init();
    rain_init();
    //light_init();
    //logger_init();
    //transmitter_init();
}

int main(void)
{
    boot();
	//puts_P(PSTR("Sensor board v0.1.1, " __TIMESTAMP__));
    
	gled_on();  // Indicate that the board is active.
    while (1) {
        process_execute_loop();
        
        gled_off();
        cpu_try_sleep();
        gled_on();
    }
}

