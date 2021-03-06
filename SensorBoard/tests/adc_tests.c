/*
 * adc_tests.cpp
 *
 * Created: 2013-12-29 22:04:58
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>
#include <stddef.h>

#include "adc_tests.h"
#include "../core/board.h"
#include "../core/cpu.h"
#include "../drivers/anemometer_driver.h"
#include "../drivers/raingauge_driver.h"
#include "../drivers/vane_driver.h"
#include "../device/adc.h"

//ADC_t *adc = &ADCA;

#define SAMPLE_DELAY	100

uint16_t lastWind, lastRain, lastBtn;
void irq_tests_setup()
{
	lastWind = 0;
	lastRain = 0;
	
	anemometer_init();
	raingauge_init();
}

bool irq_tests()
{
	if (anemometer_counter() != lastWind) {
		lastWind = anemometer_counter();
		printf_P(PSTR("Wind: %d\n"), lastWind);
	}
	if (raingauge_counter() != lastRain) {
		lastRain = raingauge_counter();
		printf_P(PSTR("Rain: %d\n"), lastRain);
	}
    return true;
}

void adc_tests_setup() 
{
	adc_setup();
}

bool adc_tests() 
{	
	puts_P(PSTR("ADC tests"));
	adc_enable();
	
	// Enable VBAT measurement
	ven_enable();
	_delay_us(5);
	
	// Measure V0REF
	uint16_t v0ref = adc_read_v0ref();
	printf_P(PSTR("VOREF %d\n"), v0ref);

	// Measure VBAT
	uint16_t vbat = adc_read(ADC_CH_MUXPOS_PIN5_gc);
	printf_P(PSTR("VBAT %d, %dmV\n"), vbat, (uint16_t)((float)(vbat-v0ref)*3.0f*2.5f/4.096f));

	// Measure VOUT
	uint16_t vout = adc_read(ADC_CH_MUXPOS_PIN4_gc);
	printf_P(PSTR("VOUT %d, %dmV\n"), vout, (uint16_t)((float)(vout-v0ref)*2.0f*2.5f/4.096f));

	// Disabled VBAT measurement
	ven_disable();

	// Measure VANE
	uint16_t vane = adc_read(ADC_CH_MUXPOS_PIN1_gc);
    int16_t diff;
	int8_t index = vane_parseReading(vane, &diff);
	printf_P(PSTR("VANE = %d (%d) diff %d\n\n"), vane, index, diff);

	adc_disable();
	return true;


}