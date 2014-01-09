/*
 * adc.c
 *
 * Created: 2014-01-09 16:54:30
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <stddef.h>

#include "adc.h"
#include "../core/cpu.h"
#include "../core/board.h"

ADC_t *adc = &ADCA;
uint16_t V0REF;

void adc_setup() 
{
	// Read ADC callibration from Signature row using NVM
	adc->CALL = cpu_read_production_signature_byte (offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	adc->CALH = cpu_read_production_signature_byte (offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
	
	// Setup ADC
	adc->CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CURRLIMIT_MEDIUM_gc;
	adc->REFCTRL = ADC_REFSEL_AREFA_gc;
	adc->PRESCALER = ADC_PRESCALER_DIV32_gc;
	adc->CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;	
}

void adc_enable() 
{
	// Enable ADC VREF power
	vrefen_enable();
	_delay_us(5);	// Wait a short while for it to stabilize (not needed?)
	
	adc->CTRLA |= _BV(0);	// Enable ADC
}

void adc_disable()
{
	// Disable ADC
	adc->CTRLA &= ~_BV(0);	// Disable ADC
	
	// Disable ADC VREF power
	vrefen_disable();
}

uint16_t adc_read (uint8_t pin)
{
	ADC_CH_t *ch = &(adc->CH0);
	ch->MUXCTRL = pin;
	ch->CTRL |= _BV(7); // Start conversion on CH0. Same as adc->CTRLA |= _BV(2);
	// We could do a delay here, but let's wait for signal instead. Test this!
	loop_until_bit_is_set(ch->INTFLAGS, 0);
	uint16_t result = ch->RES;
	ch->INTFLAGS |= _BV(0);	// Clear the flag by writing a 1 to it.
	return result;
}

uint16_t adc_read_v0ref() 
{
	// Measure V0REF
	V0REF = adc_read(ADC_CH_MUXPOS_PIN3_gc);
	return V0REF;
}

