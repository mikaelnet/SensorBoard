/*
 * i2c_driver.cpp
 *
 * Created: 2013-12-29 11:31:04
 *  Author: mikael
 */ 

#if BMP085_ENABLE == 1 || MCP79410_ENABLE == 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "i2c_driver.h"

TWI_Master_t i2cMaster;    /*!< TWI master module. */

/*! TWIC Master Interrupt vector. */
ISR(TWIE_TWIM_vect)
{
	TWI_MasterInterruptHandler(&i2cMaster);
}

void TWI_wait()
{
	while (i2cMaster.status != TWIM_STATUS_READY)
		;
}

static bool i2c_initialized = false;
void i2c_init() 
{
	if (i2c_initialized)
		return;
		
	// Initialize TWI master.
	TWI_MasterInit(&i2cMaster, &TWIE, TWI_MASTER_INTLVL_LO_gc, TWI_BAUD(F_CPU, 400000));
	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	
	i2c_initialized = true;
}

#endif