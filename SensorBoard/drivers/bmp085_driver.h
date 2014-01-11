/*
 * bmp085_driver.h
 *
 * Created: 2013-12-29 10:21:20
 *  Author: mikael
 */ 


#ifndef BMP085_DRIVER_H_
#define BMP085_DRIVER_H_

#if BMP085_ENABLE==1

#include <stdint.h>
#include <stdbool.h>

#include "twi_master_driver.h"

#define BMP085_I2CADDR           0x77

#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)

#define BMP085_CONTROL           0xF4
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6

#define BMP085_READTEMPCMD       0x2E
#define BMP085_READPRESSURECMD   0x34

#define MSLP                    101325          // Mean Sea Level Pressure = 1013.25 hPA (1hPa = 100Pa = 1mbar)

typedef enum BMP085_Mode_enum
{
	UltraLowPower = 0,
	Standard = 1,
	Highres = 2,
	UltraHighres = 3
} BMP085_Mode_t;

typedef struct BMP085_struct 
{
	TWI_Master_t *twi;
	int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
	uint16_t ac4, ac5, ac6;
	BMP085_Mode_t oversampling;
} BMP085_t;

extern bool BMP085_init(BMP085_t *bmp085, BMP085_Mode_t mode, TWI_Master_t *twi);  // by default go highres
extern float BMP085_readTemperature(BMP085_t *bmp085);
extern int32_t BMP085_readPressure(BMP085_t *bmp085);
extern float BMP085_readAltitude(BMP085_t *bmp085, float sealevelPressure); // std atmosphere,  101325 at sea level
extern uint16_t BMP085_readRawTemperature(BMP085_t *bmp085);
extern uint32_t BMP085_readRawPressure(BMP085_t *bmp085);


#endif

#endif /* BMP085_DRIVER_H_ */