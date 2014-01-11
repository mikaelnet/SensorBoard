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

static uint8_t read8(TWI_Master_t *twi, uint8_t a)
{
	uint8_t ret;
	uint8_t buffer[2];
	buffer[0] = a;

	TWI_MasterWriteRead(twi, BMP085_I2CADDR, buffer, 1, 1);
	TWI_MasterWait(twi);

	ret = twi->readData[0];
	return ret;
}

static uint16_t read16(TWI_Master_t *twi, uint8_t a) 
{
	uint16_t ret;
	uint8_t buffer[2];
	buffer[0] = a;
	
	TWI_MasterWriteRead(twi, BMP085_I2CADDR, buffer, 1, 2);
	TWI_MasterWait(twi);

	ret = (twi->readData[0] << 8) | twi->readData[1];
	return ret;
}

static void write8(TWI_Master_t *twi, uint8_t a, uint8_t d) 
{
	uint8_t buffer[2];
	buffer[0] = a;
	buffer[1] = d;
	
	TWI_MasterWrite(twi, BMP085_I2CADDR, buffer, 2);
	TWI_MasterWait(twi);
}


/*********************************************************************/


bool BMP085_init(BMP085_t *bmp085, BMP085_Mode_t mode, TWI_Master_t *twi) 
{
	bmp085->twi = twi;
	bmp085->oversampling = mode;

	if (read8(twi, 0xD0) != 0x55)	// TODO: Add constants?
		return false;

	/* read calibration data */
	bmp085->ac1 = read16(twi, BMP085_CAL_AC1);
	bmp085->ac2 = read16(twi, BMP085_CAL_AC2);
	bmp085->ac3 = read16(twi, BMP085_CAL_AC3);
	bmp085->ac4 = read16(twi, BMP085_CAL_AC4);
	bmp085->ac5 = read16(twi, BMP085_CAL_AC5);
	bmp085->ac6 = read16(twi, BMP085_CAL_AC6);

	bmp085->b1 = read16(twi, BMP085_CAL_B1);
	bmp085->b2 = read16(twi, BMP085_CAL_B2);

	bmp085->mb = read16(twi, BMP085_CAL_MB);
	bmp085->mc = read16(twi, BMP085_CAL_MC);
	bmp085->md = read16(twi, BMP085_CAL_MD);
	return true;
}

uint16_t BMP085_readRawTemperature(BMP085_t *bmp085) 
{
	write8(bmp085->twi, BMP085_CONTROL, BMP085_READTEMPCMD);
	_delay_ms(15);
	return read16(bmp085->twi, BMP085_TEMPDATA);
}

uint32_t BMP085_readRawPressure(BMP085_t *bmp085) 
{
	TWI_Master_t *twi = bmp085->twi;
	uint32_t raw;
	uint8_t oversampling = (uint8_t)(bmp085->oversampling);
	
	write8(twi, BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

	if (oversampling == UltraLowPower) 
		_delay_ms(5);
	else if (oversampling == Standard) 
		_delay_ms(8);
	else if (oversampling == Highres) 
		_delay_ms(14);
	else 
		_delay_ms(26);

	raw = read16(twi, BMP085_PRESSUREDATA);
	raw <<= 8;
	raw |= read8(twi, BMP085_PRESSUREDATA+2);
	raw >>= (8 - oversampling);

	return raw;
}


int32_t BMP085_readPressure(BMP085_t *bmp085) 
{
	uint16_t UT;
	uint32_t UP;
	int32_t B3, B5, B6, X1, X2, X3, p;
	uint32_t B4, B7;

	UT = BMP085_readRawTemperature(bmp085);
	UP = BMP085_readRawPressure(bmp085);

	printf_P(PSTR("Raw temp/pres: %10.2f / %10.2f\n"), (double)UT, (double)UP);

	// do temperature calculations
	X1=(UT-(int32_t)(bmp085->ac6))*((int32_t)(bmp085->ac5))/pow(2,15);
	X2=((int32_t)bmp085->mc*pow(2,11))/(X1+(int32_t)bmp085->md);
	B5=X1 + X2;

	// do pressure calcs
	B6 = B5 - 4000;
	X1 = ((int32_t)bmp085->b2 * ( (B6 * B6)>>12 )) >> 11;
	X2 = ((int32_t)bmp085->ac2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)bmp085->ac1*4 + X3) << (uint8_t)bmp085->oversampling) + 2) / 4;

	X1 = ((int32_t)bmp085->ac3 * B6) >> 13;
	X2 = ((int32_t)bmp085->b1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = ((uint32_t)bmp085->ac4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> (uint8_t)bmp085->oversampling );

	if (B7 < 0x80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}
	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;

	p = p + ((X1 + X2 + (int32_t)3791)>>4);
	return p;
}


float BMP085_readTemperature(BMP085_t *bmp085) 
{
	int32_t UT, X1, X2, B5;     // following ds convention
	float temp;

	UT = BMP085_readRawTemperature(bmp085);

	// step 1
	X1 = (UT - (int32_t)bmp085->ac6) * ((int32_t)bmp085->ac5) / pow(2,15);
	X2 = ((int32_t)bmp085->mc * pow(2,11)) / (X1+(int32_t)bmp085->md);
	B5 = X1 + X2;
	temp = (B5+8)/pow(2,4);
	temp /= 10;
  
	return temp;
}

float BMP085_readAltitude(BMP085_t *bmp085, float sealevelPressure) 
{
	float pressure = BMP085_readPressure(bmp085);
	float altitude = 44330 * (1.0 - pow(pressure /sealevelPressure,0.1903));
	return altitude;
}


#endif