/*
 * vane_driver.cpp
 *
 * Created: 2013-12-28 11:04:51
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

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

#define RBASE	5600.0
#define R0	33000.0
#define R1	8200.0
#define R2	1000.0
#define R3	2200.0
#define R4	3900.0
#define R5	16000.0
#define R6	120000.0
#define R7	64900.0

#define RPAIR(a,b)	(1.0/(1.0/(a)+1.0/(b)))
#define RADC(a)	(4096*(a)/((a)+RBASE))

typedef struct WindDirection_struct {
	uint8_t index;
	uint16_t adcValue;
	uint16_t adcThreshold;
} WindDirection_t;

static int16_t minAdcValue;
static WindDirection_t WindDirections[] = 
{
	{  0, RADC(R0) },
	{  1, RADC(RPAIR(R0, R1)) },
	{  2, RADC(R1) },
	{  3, RADC(RPAIR(R1, R2)) },
	{  4, RADC(R2) },
	{  5, RADC(RPAIR(R2, R3)) },
	{  6, RADC(R3) },
	{  7, RADC(RPAIR(R3, R4)) },
	{  8, RADC(R4) },
	{  9, RADC(RPAIR(R4, R5)) },
	{ 10, RADC(R5) },
	{ 11, RADC(RPAIR(R5, R6)) },
	{ 12, RADC(R6) },
	{ 13, RADC(RPAIR(R6, R7)) },
	{ 14, RADC(R7) },
	{ 15, RADC(RPAIR(R7, R0)) },
};

void vane_init () 
{
	//for (int i=0 ; i < 16 ; i ++)
	//	printf_P(PSTR("%d %d %d\n"), i, WindDirections[i].index, WindDirections[i].adcValue);

	// Sort the list
	for (int8_t i=0; i < 16; i++) {
		for (int8_t j=i+1; j < 16; j++) {
			if (WindDirections[j].adcValue < WindDirections[i].adcValue) {
				// Swap elements
				uint8_t index = WindDirections[i].index;
				WindDirections[i].index = WindDirections[j].index;
				WindDirections[j].index = index;
				
				uint16_t adc = WindDirections[i].adcValue;
				WindDirections[i].adcValue = WindDirections[j].adcValue;
				WindDirections[j].adcValue = adc;
			}
		}
	}

	for (int8_t i=0 ; i < 15 ; i ++) {
		WindDirections[i].adcThreshold = (WindDirections[i].adcValue + WindDirections[i+1].adcValue)/2;
	}
	// Treat ADC values above this value as a disconnected sensor
	WindDirections[15].adcThreshold = (WindDirections[15].adcValue + 4096)/2;
	// Treat ADC values below this value as a shortcut sensor
	minAdcValue = WindDirections[0].adcValue / 2;

	//puts_P(PSTR("\nSorted:"));
	//for (int i=0 ; i < 16 ; i ++)
	//	printf_P(PSTR("%d %d %d %d\n"), i, WindDirections[i].index, WindDirections[i].adcValue, 
	//	WindDirections[i].adcThreshold);
}


int8_t vane_parseReading (uint16_t reading, int16_t *diff)
{
	// Shortcut
	if (reading < minAdcValue) {
		*diff = reading - minAdcValue;
        return -1;
    }        

	for (int8_t i=0 ; i < 16 ; i ++) {
		if (reading < WindDirections[i].adcThreshold) {
            *diff = reading - WindDirections[i].adcValue;
			return WindDirections[i].index;
        }
	}
	
	// Disconnected
    *diff = reading - WindDirections[15].adcThreshold;
	return -1;
}

