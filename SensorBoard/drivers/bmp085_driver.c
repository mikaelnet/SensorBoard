/*
 * bmp085_driver.cpp
 *
 * Created: 2013-12-29 10:20:41
 *  Author: mikael
 */ 

#if BMP085_ENABLE==1


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>

#include "bmp085_driver.h"
#include "twi_master_driver.h"

static int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
static uint16_t ac4, ac5, ac6;
static bmp085_Mode_t _oversampling;

TWI_Master_t *_twi;

static uint8_t read8(uint8_t a)
{
	uint8_t ret;
	uint8_t buffer[2];
	buffer[0] = a;

	TWI_MasterWriteRead(_twi, BMP085_I2CADDR, buffer, 1, 1);
	TWI_wait(_twi);

	ret = _twi->readData[0];
	return ret;
}

static uint16_t read16(uint8_t a) 
{
	uint16_t ret;
	uint8_t buffer[2];
	buffer[0] = a;
	
	TWI_MasterWriteRead(_twi, BMP085_I2CADDR, buffer, 1, 2);
	TWI_wait(_twi);

	ret = (_twi->readData[0] << 8) | _twi->readData[1];
	return ret;
}

static void write8(uint8_t a, uint8_t d) 
{
	uint8_t buffer[2];
	buffer[0] = a;
	buffer[1] = d;
	
	TWI_MasterWrite(_twi, BMP085_I2CADDR, buffer, 2);
	TWI_wait(_twi);
}


/*********************************************************************/


bool BMP085_begin(bmp085_Mode_t mode, TWI_Master_t *twi) 
{
	_twi = twi;
	_oversampling = mode;

	if (read8(0xD0) != 0x55)	// TODO: Add constants?
		return false;

	/* read calibration data */
	ac1 = read16(BMP085_CAL_AC1);
	ac2 = read16(BMP085_CAL_AC2);
	ac3 = read16(BMP085_CAL_AC3);
	ac4 = read16(BMP085_CAL_AC4);
	ac5 = read16(BMP085_CAL_AC5);
	ac6 = read16(BMP085_CAL_AC6);

	b1 = read16(BMP085_CAL_B1);
	b2 = read16(BMP085_CAL_B2);

	mb = read16(BMP085_CAL_MB);
	mc = read16(BMP085_CAL_MC);
	md = read16(BMP085_CAL_MD);
	return true;
}

uint16_t BMP085_readRawTemperature(void) 
{
	write8(BMP085_CONTROL, BMP085_READTEMPCMD);
	_delay_ms(15);
	return read16(BMP085_TEMPDATA);
}

uint32_t BMP085_readRawPressure(void) 
{
	uint32_t raw;

	write8(BMP085_CONTROL, BMP085_READPRESSURECMD + ((uint8_t)_oversampling << 6));

	if (_oversampling == UltraLowPower) 
		_delay_ms(5);
	else if (_oversampling == Standard) 
		_delay_ms(8);
	else if (_oversampling == Highres) 
		_delay_ms(14);
	else 
		_delay_ms(26);

	raw = read16(BMP085_PRESSUREDATA);
	raw <<= 8;
	raw |= read8(BMP085_PRESSUREDATA+2);
	raw >>= (8 - (uint8_t)_oversampling);

	return raw;
}


int32_t BMP085_readPressure(void) 
{
	int32_t UT;
	int32_t UP;
	int32_t B3, B5, B6, X1, X2, X3, p;
	uint32_t B4, B7;

	UT = BMP085_readRawTemperature();
	UP = BMP085_readRawPressure();

#if BMP085_DEBUG == 1
	// use datasheet numbers!
	UT = 27898;
	UP = 23843;
	ac6 = 23153;
	ac5 = 32757;
	mc = -8711;
	md = 2868;
	b1 = 6190;
	b2 = 4;
	ac3 = -14383;
	ac2 = -72;
	ac1 = 408;
	ac4 = 32741;
	oversampling = 0;
	
	printf_P(PSTR("Raw temp: %ld\n"), UT);
	printf_P(PSTR("Raw pres: %ld\n"), UP);
	printf_P(PSTR("ac1 = %d\n"), ac1);
	printf_P(PSTR("ac2 = %d\n"), ac2);
	printf_P(PSTR("ac3 = %d\n"), ac3);
	printf_P(PSTR("ac4 = %d\n"), ac4);
	printf_P(PSTR("ac5 = %d\n"), ac5);
	printf_P(PSTR("ac6 = %d\n\n"), ac6);

	printf_P(PSTR("b1 = %d\n"), b1);
	printf_P(PSTR("b2 = %d\n\n"), b2);

	printf_P(PSTR("mb = %d\n"), mb);
	printf_P(PSTR("mc = %d\n"), mc);
	printf_P(PSTR("md = %d\n"), md);
#endif

	// do temperature calculations
	X1=(UT-(int32_t)(ac6))*((int32_t)(ac5))/pow(2,15);
	X2=((int32_t)mc*pow(2,11))/(X1+(int32_t)md);
	B5=X1 + X2;

#if BMP085_DEBUG == 1
	printf_P(PSTR("X1 = %d\n"), X1);
	printf_P(PSTR("X2 = %d\n"), X2);
	printf_P(PSTR("B5 = %d\n"), B5);
#endif

	// do pressure calcs
	B6 = B5 - 4000;
	X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
	X2 = ((int32_t)ac2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)ac1*4 + X3) << (uint8_t)_oversampling) + 2) / 4;

#if BMP085_DEBUG == 1
	printf_P(PSTR("B6 = %d\n"), B6);
	printf_P(PSTR("X1 = %d\n"), X1);
	printf_P(PSTR("X2 = %d\n"), X2);
	printf_P(PSTR("B3 = %d\n"), B3);
#endif

	X1 = ((int32_t)ac3 * B6) >> 13;
	X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> (uint8_t)_oversampling );

#if BMP085_DEBUG == 1
	printf_P(PSTR("X1 = %d\n"), X1);
	printf_P(PSTR("X2 = %d\n"), X2);
	printf_P(PSTR("B4 = %d\n"), B4);
	printf_P(PSTR("B7 = %d\n"), B7);
#endif

	if (B7 < 0x80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;

#if BMP085_DEBUG == 1
	printf_P(PSTR("p = %d\n"), p);
	printf_P(PSTR("X1 = %d\n"), X1);
	printf_P(PSTR("X2 = %d\n"), X2);
#endif

	p = p + ((X1 + X2 + (int32_t)3791)>>4);
#if BMP085_DEBUG == 1
	printf_P(PSTR("p = %d\n", p);
#endif
	return p;
}


float BMP085_readTemperature(void) 
{
	int32_t UT, X1, X2, B5;     // following ds convention
	float temp;

	UT = BMP085_readRawTemperature();

#if BMP085_DEBUG == 1
	// use datasheet numbers!
	UT = 27898;
	ac6 = 23153;
	ac5 = 32757;
	mc = -8711;
	md = 2868;
#endif

	// step 1
	X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) / pow(2,15);
	X2 = ((int32_t)mc * pow(2,11)) / (X1+(int32_t)md);
	B5 = X1 + X2;
	temp = (B5+8)/pow(2,4);
	temp /= 10;
  
	return temp;
}

float BMP085_readAltitude(float sealevelPressure) 
{
	float pressure = BMP085_readPressure();
	float altitude = 44330 * (1.0 - pow(pressure /sealevelPressure,0.1903));
	return altitude;
}


#endif