/*
 * cpu.c
 *
 * Created: 2013-12-28 00:30:34
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stddef.h>

#include "cpu.h"
#include "console.h"

typedef enum CPUSpeed_enum {
	CLOCK_2MHz,
	CLOCK_32MHz
} CPUSpeed_t;

static CPUSpeed_t _cpuSpeed;

static CPU_SleepMethod_t *sleepMethods = NULL;

void cpu_init()
{
    // Clear watchdog at startup, because it may be running.
    wdt_reset();

    #if F_CPU == 32000000UL
    cpu_set_32_MHz();
    #elif F_CPU == 2000000UL
    cpu_set_2_MHz();
    #else
    #error "Unknown or non supported F_CPU value"
    #endif
    cpu_init_timer();

    // Disable some functions for power saving
    power_usb_disable();
    PR.PRGEN = _BV(6) | _BV(4) | _BV(3) | _BV(2) | _BV(1) | _BV(0); // Power reduction: USB, AES, EBI, RTC, EVSYS, DMA

    wdt_reset();
}

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
static volatile uint16_t _seconds;

ISR(TCD0_OVF_vect)
{
    uint16_t ms = _milliseconds;
    ms ++;
    _milliseconds = ms;
    if (ms % 1000 == 0)
        _seconds ++;
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

uint16_t cpu_second()
{
    uint16_t timer;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        timer = _seconds;
    }
    return timer;
}

void cpu_register_sleep_methods(CPU_SleepMethod_t *holder, 
    bool (*canSleepMethod)(),
    void (*beforeSleepMethod)(), 
    void (*afterWakeupMethod)())
{
    holder->canSleepMethod = canSleepMethod;
    holder->beforeSleepMethod = beforeSleepMethod;
    holder->afterWakeupMethod = afterWakeupMethod;
    holder->next = sleepMethods;
    sleepMethods = holder;
}

bool cpu_can_sleep()
{
    // loop through all registered can sleep methods. If any of them returns false, don'd do anything.
    CPU_SleepMethod_t *ptr = sleepMethods;
    while (ptr) {
        if (ptr->canSleepMethod != NULL) {
            bool result = (ptr->canSleepMethod)();
            if (!result)
                return false;
        }
        ptr = ptr->next;
    }
    return true;
}

void cpu_sleep()
{
    // Disable normal UART operation and enable wake on (RX) pin change
    //console_disable();
    CPU_SleepMethod_t *ptr = sleepMethods;
    while (ptr) {
        if (ptr->beforeSleepMethod != NULL) {
            (*ptr->beforeSleepMethod)();
        }
        ptr = ptr->next;
    }
    
    // Disable watch dog before going to sleep, because WDT is running while in sleep
    wdt_reset();
    //wdt_disable();
    
    // Go to power save mode
    SLEEP.CTRL = SLEEP_SMODE_STDBY_gc | SLEEP_SEN_bm; // SLEEP_SMODE_PDOWN_gc  / SLEEP_SMODE_PSAVE_gc
    sei();	// Force interrupts on, otherwise the device cannot wake up again.
    sleep_cpu();
    SLEEP.CTRL = 0; // Go back to normal operation
    
    // Re-enable normal UART operation
    //console_enable();
    ptr = sleepMethods;
    while (ptr) {
        if (ptr->afterWakeupMethod != NULL) {
            (*ptr->afterWakeupMethod)();
        }
        ptr = ptr->next;
    }
}

void cpu_try_sleep()
{
    if (cpu_can_sleep())
        cpu_sleep();
}

