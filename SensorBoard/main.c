/*
 * SensorBoard.cpp
 *
 * Created: 2013-12-27 22:14:11
 *  Author: mikael
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>


#include "core/cpu.h"
#include "core/console.h"
#include "core/board.h"

#include "application/terminal.h"
#include "application/clock.h"
#include "application/thermometer.h"

#include "tests/dht22_tests.h"
#include "tests/ds1820_tests.h"
#include "tests/bmp085_tests.h"
#include "tests/adc_tests.h"
#include "tests/mcp79410_tests.h"
#include "tests/sd_tests.h"

#include "drivers/vane_driver.h"

void setup()
{
	// Enable 
	
#if F_CPU == 32000000UL
	cpu_set_32_MHz();
#elif F_CPU == 2000000UL
	cpu_set_2_MHz();
#else
	#error "Unknown or non supported F_CPU value"
#endif
	cpu_init_timer();
	init_board();
	console_init();
	sei();
    terminal_init();
    clock_init();
    thermometer_init();
    
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
	adc_tests_setup();
	irq_tests_setup();
	vane_init();
    //sd_tests_setup();
}

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

void goToSleep () {
    gled_off();
    cpu_sleep();
    gled_on();
}

#if 0
char consoleBuffer[80];
char *consoleBufPtr;
void console_test() {
    consoleBufPtr = consoleBuffer;
    puts_P(PSTR("Write something..."));
    
    uint16_t timer = cpu_millisecond();
    gled_on();
    while (1) {
        
        if (console_hasdata()) {
            timer = cpu_millisecond();  // Reset timer
            char ch = fgetc(stdin);
            if (ch == 0x08) {
                consoleBufPtr--;
                *consoleBufPtr = 0;
            }
            else if (ch == '\r' || ch == '\n') {
                *consoleBufPtr = 0;
                if (strcasecmp_P(consoleBuffer, PSTR("exit")) == 0)
                    break;
                    
                puts("You wrote:");
                puts(consoleBuffer);
                consoleBufPtr = consoleBuffer;
            }
            else            
                *consoleBufPtr++ = ch;
        }
        
        if (cpu_millisecond() - timer > 5000) {
            goToSleep();
        }
    }
}
#endif

int main(void)
{
	// Clear watchdog at startup, because it may be running.
	wdt_reset();
	
	setup();
    sei();
	puts_P(PSTR("Sensor board v0.1.1, " __TIMESTAMP__));
	printf_P(PSTR("Running at %d MHz\n"), F_CPU/1000000UL);
    
	gled_on();  // Indicate that the board is active.
    
	// Disable some functions for power saving
	power_usb_disable();
	PR.PRGEN = _BV(6) | _BV(4) | _BV(3) | _BV(2) | _BV(1) | _BV(0); // Power reduction: USB, AES, EBI, RTC, EVSYS, DMA

    while (1) {
        terminal_process();
        if (terminal_can_sleep())
            goToSleep();
    }


    //console_test();

	// - test avr/power.h!!
    while(1)
    {
		// Enable watch dog here
		//wdt_enable(WDTO_1S);	// Vet ej om detta blir rätt på xmega
		wdt_reset();


		bool canSleep = loop();

		//uint16_t start = cpu_millisecond();
		//_delay_ms(123);
		//uint16_t totalTime = cpu_millisecond() - start;
		//printf_P(PSTR("Loop time %d ms (expected ~123ms)\n"), totalTime);


	    if (canSleep)
            goToSleep();	
    }
}