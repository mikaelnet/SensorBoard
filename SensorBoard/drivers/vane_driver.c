/*
 * vane_driver.cpp
 *
 * Created: 2013-12-28 11:04:51
 *  Author: mikael
 */ 

#include <avr/io.h>
#include "vane_driver.h"

/*
	Resistor values:
	0 deg	33k
	45 deg	8k2
	90 deg	1k
	135 deg	2k2
	180 deg	3k9
	225 deg	16k
	270 deg 120k
	315 deg	64k9
	
	Indexes 0-15, 0=0 deg, 15=337.5 deg
	Index ordered by resistor value:
	5, 3, 4, 7, 6, 9, 8, 1, 2, 11, 10, 15, 0, 13, 14, 12
*/

// These values are taken from calculated excel sheet
#define ADCR_I0		3502	// 0 deg
#define ADCR_I1		2211	// 22.5 deg
#define ADCR_I2		2434	// 45 deg
#define ADCR_I3		 562	// 67.2 deg
#define ADCR_I4		 621	// 90 deg
#define ADCR_I5		 448	// 112.5 deg
#define ADCR_I6		1155	// 135 deg
#define ADCR_I7		 822	// 157.5 deg
#define ADCR_I8		1682	// 180 deg
#define ADCR_I9		1470	// 202.5 deg
#define ADCR_I10	3034	// 225 deg
#define ADCR_I11	2933	// 247.5 deg
#define ADCR_I12	3913	// 270 deg
#define ADCR_I13	3615	// 292.5 deg
#define ADCR_I14	3771	// 315 deg
#define ADCR_I15	3261	// 337.5 deg


/*static*/ uint8_t parseReading (uint16_t reading)
{
	// Optimized by halving index
	if (reading <= (ADCR_I1+ADCR_I2)/2) {
		if (reading <= (ADCR_I6+ADCR_I7)/2) {
			if (reading <= (ADCR_I3+ADCR_I4)/2) {
				return reading <= (ADCR_I3+ADCR_I5)/2 ? 5 : 3;
			}
			else {
				return reading <= (ADCR_I4+ADCR_I7)/2 ? 4 : 7;
			}
		}
		else {
			if (reading <= (ADCR_I8+ADCR_I9)/2) {
				return reading <= (ADCR_I6+ADCR_I9)/2 ? 6 : 9;
			}
			else {
				return reading <= (ADCR_I1+ADCR_I8)/2 ? 8 : 1;
			}
		}
	}
	else {
		if (reading <= (ADCR_I0+ADCR_I15)/2) {
			if (reading <= (ADCR_I10+ADCR_I11)/2) {
				return reading <= (ADCR_I2+ADCR_I11)/2 ? 2 : 11;
			}
			else {
				return reading <= (ADCR_I10+ADCR_I15)/2 ? 10 : 15;
			}
		}
		else {
			if (reading <= (ADCR_I13+ADCR_I14)/2) {
				return reading <= (ADCR_I0+ADCR_I13)/2 ? 0 : 13;
			}
			else {
				return reading <= (ADCR_I14+ADCR_I12)/2 ? 14 : 12;
			}
		}
	}
}