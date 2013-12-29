/*
 * bmp085_tests.cpp
 *
 * Created: 2013-12-29 10:14:08
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "../drivers/bmp085_driver.h"
BMP085 bmp085;

#ifdef __cplusplus
extern "C" {
#endif

void bmp085_tests() 
{
	puts_P(PSTR("Testing BMP085..."));
	bmp085.begin(BMP085_STANDARD);
	float temperature = bmp085.readTemperature();
	int32_t pressure = bmp085.readPressure();
	printf_P(PSTR("BMP085 Temperature: %f\n"), temperature);
	printf_P(PSTR("BMP085 Pressure: %d\n"), pressure);
}

#ifdef __cplusplus
}
#endif
