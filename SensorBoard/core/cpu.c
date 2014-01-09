/*
 * cpu.c
 *
 * Created: 2013-12-28 00:30:34
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stddef.h>
#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CPUSpeed_enum {
	CLOCK_2MHz,
	CLOCK_32MHz
} CPUSpeed_t;

static CPUSpeed_t _cpuSpeed;

void cpu_set_32_MHz()
{
	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	OSC.CTRL = OSC_RC32MEN_bm;       // enable 32MHz oscillator
	
	while(!(OSC.STATUS & OSC_RC32MRDY_bm))
		;							 // wait for oscillator to be ready
	
	CCP = CCP_IOREG_gc;              // disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
	_cpuSpeed = CLOCK_32MHz;
}

void cpu_set_2_MHz()
{
	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	OSC.CTRL = OSC_RC2MEN_bm;        // enable 2MHz oscillator
	
	while(!(OSC.STATUS & OSC_RC2MRDY_bm))
		;							 // wait for oscillator to be ready
	
	CCP = CCP_IOREG_gc;              // disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC2M_gc;  // switch to 2MHz clock
	_cpuSpeed = CLOCK_2MHz;
}

uint8_t cpu_read_production_signature_byte (uint8_t index)
{
	// Load the NVM Command register to read the calibration row.
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	uint8_t result = pgm_read_byte(index);
	// Clean up NVM Command register.
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	return result;
}

static volatile uint16_t _milliseconds;

ISR(TCD0_OVF_vect)
{
	_milliseconds ++;
}

void TC0_ConfigClockSource( volatile TC0_t * tc, TC_CLKSEL_t clockSelection )
{
	tc->CTRLA = ( tc->CTRLA & ~TC0_CLKSEL_gm ) | clockSelection;
}

void cpu_init_timer()
{
	if (_cpuSpeed == CLOCK_2MHz) {
		TC0_ConfigClockSource(&TCC0, TC_CLKSEL_DIV4_gc);	// Tick once every 2MHz/4=2us
		TC0_ConfigClockSource(&TCD0, TC_CLKSEL_DIV8_gc);	// Tick once every 2MHz/8=4us
		TCD0.PER = 1000/4;
	}
	else {
		// 32MHz
		TC0_ConfigClockSource(&TCC0, TC_CLKSEL_DIV64_gc);	// Tick once every 32MHz/64=2us
		TC0_ConfigClockSource(&TCD0, TC_CLKSEL_DIV256_gc);	// Tick once every 32MHz/256=8us
		TCD0.PER = 1000/8;
	}
	TCD0.CNT = 0;
	//TCD0.CNT = 1000/8;
	//TCD0.CTRLFSET |= _BV(0);
	TCD0.INTCTRLA |= TC_OVFINTLVL_LO_gc;
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
}

uint16_t cpu_microsecond()
{
	uint16_t timer;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer = TCC0.CNT;
	}
	return timer;
}

uint16_t cpu_millisecond()
{
	uint16_t timer;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		timer = _milliseconds;
	}
	return timer;
}

#ifdef __cplusplus
}
#endif