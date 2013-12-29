/*
 * i2c_driver.cpp
 *
 * Created: 2013-12-29 11:31:04
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "i2c_driver.h"

I2C::I2C (TWI_t *twi, uint8_t baudRateRegisterSetting)
{
	_twi = twi;
	twi->MASTER.CTRLA = TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_ENABLE_bm;
	twi->MASTER.BAUD = baudRateRegisterSetting;
	twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

void I2C::write (uint8_t data)
{
	
}

void I2C::write(uint8_t *data, int8_t length)
{
	while (length > 0) {
		write(*data ++);
		length --;
	}
}
