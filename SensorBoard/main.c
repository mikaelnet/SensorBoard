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
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>


#include "core/cpu.h"
#include "core/console.h"
#include "core/board.h"


#include "tests/dht22_tests.h"
#include "tests/ds1820_tests.h"
#include "tests/bmp085_tests.h"
#include "tests/adc_tests.h"
#include "tests/mcp79410_tests.h"

#include "drivers/vane_driver.h"

void setup()
{
#if F_CPU == 32000000UL
	cpu_set_32_MHz();
#endif
	cpu_init_timer();
	init_board();
	console_init();
	
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
}

void loop()
{
#if DHT22_ENABLE==1
	dht22_tests();
#endif
#if DS1820_ENABLE==1
	ds1820_tests();
#endif
#if BMP085_ENABLE==1	
	bmp085_tests();
#endif
#if MCP79410_ENABLE==1
	mcp79410_tests();
#endif
	adc_tests();
	irq_tests();
	vane_init();
}

int main(void)
{
	// Clear watchdog at startup, because it may be running.
	wdt_reset();
	
	setup();
	sei();
	puts_P(PSTR("Sensor board v0.1.1, " __TIMESTAMP__));
	printf_P(PSTR("Running at %d MHz\n"), F_CPU/1000000UL);

	// Disable some functions for power saving
	power_usb_disable();
	PR.PRGEN = _BV(6) | _BV(4) | _BV(3) | _BV(2) | _BV(1) | _BV(0); // Power reduction: USB, AES, EBI, RTC, EVSYS, DMA
	// - test avr/power.h!!
    while(1)
    {
		// Enable watch dog here
		//wdt_enable(WDTO_1S);	// Vet ej om detta blir rätt på xmega
		wdt_reset();


		gled_on();
		loop();
	
		// wait for uart buffer to empty
		while (!console_txempty())
			;
		_delay_ms(5);	// Kolla också så att sista tecknet är skickat
		
		gled_off();
		
		
		// Disable watch dog before going to sleep, because wdt is running while in sleep
		wdt_reset();
		//wdt_disable();
		
		// Go to power save mode
		SLEEP.CTRL = SLEEP_SMODE_STDBY_gc | SLEEP_SEN_bm; //SLEEP_SMODE_PSAVE_gc | SLEEP_SEN_bm;
		sei();	// Force interrupts on, otherwise the device cannot wake up again.
		sleep_cpu();
		SLEEP.CTRL = 0;
    }
}