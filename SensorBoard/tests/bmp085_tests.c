/*
 * bmp085_tests.cpp
 *
 * Created: 2013-12-29 10:14:08
 *  Author: mikael
 */ 

#if BMP085_ENABLE==1

#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "../drivers/bmp085_driver.h"
#include "../device/i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

void bmp085_tests_setup() 
{
}

void bmp085_tests() 
{
	puts_P(PSTR("BMP085..."));
	BMP085_begin(Standard, &i2c);
	float temperature = BMP085_readTemperature();
	int32_t pressure = BMP085_readPressure();

	int16_t temp = floor(temperature);	

	printf_P(PSTR("BMP085 Temperature: %d%cC\n"), temp, 0xB0);
	printf_P(PSTR("BMP085 Pressure: %ld\n"), pressure);
}

#ifdef __cplusplus
}
#endif

#endif