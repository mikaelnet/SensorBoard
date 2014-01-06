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

ADC_t *adc = &ADCA;

#define SAMPLE_DELAY	100

volatile uint16_t WindCount = 0;
// WIND
ISR(PORTA_INT0_vect)
{
	WindCount ++;
	// Potential, test PORTA.INTFLAGS & 0x01;
}

volatile uint16_t RainCount = 0;
// RAIN
ISR(PORTB_INT0_vect)
{
	RainCount ++;
}

volatile uint8_t ButtonCount = 0;
// Button
ISR(PORTC_INT0_vect)
{
	ButtonCount ++;
}

uint16_t lastWind, lastRain, lastBtn;
void irq_tests_setup()
{
	lastWind = 0;
	lastRain = 0;
	lastBtn = 0;
		
	PORTA.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTA.INT0MASK = _BV(2);
	PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

	PORTB.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTB.INT0MASK = _BV(2);
	PORTB.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;

	PORTC.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTC.INT0MASK = _BV(2);
	PORTC.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
	
	
	// Enable medium level interrupts in the PMIC. 
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;
}

void irq_tests()
{
	if (WindCount != lastWind) {
		lastWind = WindCount;
		printf_P(PSTR("Wind: %d\n"), lastWind);
	}
	if (RainCount != lastRain) {
		lastRain = RainCount;
		printf_P(PSTR("Rain: %d\n"), lastRain);
	}
	if (ButtonCount != lastBtn) {
		lastBtn = ButtonCount;
		printf_P(PSTR("Button: %d\n"), lastBtn);
	}
}

void adc_tests_setup() 
{
	// Read ADC callibratoin from Signature row using NVM
	adc->CALL = cpu_read_production_signature_byte (offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	adc->CALH = cpu_read_production_signature_byte (offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
		
	// Setup ADC
	adc->CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CURRLIMIT_MEDIUM_gc;
	adc->REFCTRL = ADC_REFSEL_AREFA_gc;
	adc->PRESCALER = ADC_PRESCALER_DIV32_gc;
	adc->CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
}

void adc_tests() 
{	
	puts_P(PSTR("ADC tests"));
	// Enable ADC VREF power
	vrefen_enable();
	_delay_us(5);	
	
	adc->CTRLA |= _BV(0);	// Enable ADC
	
	// Enable VBAT measurement
	ven_enable();
	_delay_us(5);
	
	// Measure V0REF
	adc->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;
	adc->CH0.CTRL |= _BV(7); // Start conversion on CH0 ==  adc->CTRLA |= _BV(2);
	_delay_us(SAMPLE_DELAY);
	uint16_t v0ref = adc->CH0.RES;
	printf_P(PSTR("VOREF %d\n"), v0ref);

	// Measure VBAT
	adc->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;
	adc->CH0.CTRL |= _BV(7); // Start conversion on CH0 ==  adc->CTRLA |= _BV(2);
	_delay_us(SAMPLE_DELAY);
	uint16_t vbat = adc->CH0.RES;
	printf_P(PSTR("VBAT %d, %dmV\n"), vbat, (uint16_t)((float)(vbat-v0ref)*3.0f*2.5f/4.096f));

	// Measure VOUT
	adc->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc;
	adc->CH0.CTRL |= _BV(7); // Start conversion on CH0 ==  adc->CTRLA |= _BV(2);
	_delay_us(SAMPLE_DELAY);
	uint16_t vout = adc->CH0.RES;
	printf_P(PSTR("VOUT %d, %dmV\n"), vout, (uint16_t)((float)(vout-v0ref)*2.0f*2.5f/4.096f));

	// Disabled VBAT measurement
	ven_disable();

	// Measure VANE
	adc->CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
	adc->CH0.CTRL |= _BV(7); // Start conversion on CH0 ==  adc->CTRLA |= _BV(2);
	_delay_us(SAMPLE_DELAY);
	uint16_t vane = adc->CH0.RES;
	printf_P(PSTR("VANE = %d\n\n"), vane);

	// Disable ADC
	adc->CTRLA &= ~_BV(0);	// Disable ADC
	
	// Disable ADC VREF power
	vrefen_disable();
}