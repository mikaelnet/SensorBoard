/*
 * cpu.c
 *
 * Created: 2013-12-28 00:30:34
 *  Author: mikael
 */ 

#include <avr/io.h>

#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

void cpu_set_32_MHz()
{
	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	OSC.CTRL = OSC_RC32MEN_bm;       // enable 32MHz oscillator
	
	while(!(OSC.STATUS & OSC_RC32MRDY_bm))
	;							 // wait for oscillator to be ready
	
	CCP = CCP_IOREG_gc;              // disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
}

void cpu_set_2_MHz()
{
	CCP = CCP_IOREG_gc;              // disable register security for oscillator update
	OSC.CTRL = OSC_RC2MEN_bm;        // enable 2MHz oscillator
	
	while(!(OSC.STATUS & OSC_RC2MRDY_bm))
	;							 // wait for oscillator to be ready
	
	CCP = CCP_IOREG_gc;              // disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC2M_gc;  // switch to 32MHz clock
}

#ifdef __cplusplus
}
#endif