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
	
	int16_t t1 = temperature;
	int16_t t2 = (temperature * 100);
	t2 %= 100;
	
	uint16_t p1 = pressure >> 16;
	uint16_t p2 = pressure & 0xFFFF;
	printf_P(PSTR("BMP085 Temperature: %d.%02d\n"), t1, t2);
	printf_P(PSTR("BMP085 Pressure: %d:%d\n"), p1, p2);
}

#ifdef __cplusplus
}
#endif

#endif