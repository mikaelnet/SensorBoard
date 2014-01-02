/*
 * i2c_driver.h
 *
 * This is the device specific i2c bus.
 * This file makes sure it's initialized only once etc.
 * 
 * Created: 2013-12-29 11:30:55
 *  Author: mikael
 */ 


#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

#if BMP085_ENABLE == 1 || MCP79410_ENABLE == 1

#include "../drivers/twi_master_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TWI_Master_t i2c;    /*!< TWI master module. */
extern void i2c_init();

#ifdef __cplusplus
}
#endif

#endif

#endif /* I2C_DRIVER_H_ */