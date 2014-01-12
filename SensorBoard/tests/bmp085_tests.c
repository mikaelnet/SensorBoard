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
#include "../drivers/TSL2561_driver.h"
#include "../device/i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

TSL2561_t light;
BMP085_t bmp085;

void bmp085_tests_setup() 
{
	TSL2561_Init(&light, &i2c, TSL2561_ADDR_FLOAT);
}

bool bmp085_tests() 
{
	puts_P(PSTR("BMP085..."));
	BMP085_init(&bmp085, Standard, &i2c);
	float temperature = BMP085_readTemperature(&bmp085);
	int32_t pressure = BMP085_readPressure(&bmp085);

	int16_t temp = floor(temperature);	

	printf_P(PSTR("BMP085 Temperature: %d%cC\n"), temp, 0xB0);
	printf_P(PSTR("BMP085 Pressure: %ld\n"), pressure);
	
	puts_P(PSTR("TSL2561..."));
	TSL2561_begin(&light);
	uint16_t ch0 = TSL2561_getLuminosity(&light, TSL2561_FULLSPECTRUM);
	uint16_t ch1 = TSL2561_getLuminosity(&light, TSL2561_INFRARED);
	uint16_t ch2 = TSL2561_getLuminosity(&light, TSL2561_VISIBLE);
	uint32_t lux = TSL2561_calculateLux(&light, ch0, ch1);
	
	printf_P(PSTR("Full spectrum: %d\n"), ch0);
	printf_P(PSTR("Infrared: %d\n"), ch1);
	printf_P(PSTR("Visible: %d\n"), ch2);
	printf_P(PSTR("lux: %ld\n"), lux);
    
    return true;
}

#ifdef __cplusplus
}
#endif

#endif