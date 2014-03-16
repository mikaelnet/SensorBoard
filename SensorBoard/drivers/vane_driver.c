/*
 * vane_driver.cpp
 *
 * Created: 2013-12-28 11:04:51
 *  Author: mikael
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <math.h>

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

#define VANE_RBASE	5600.0
#define VANE_R0	33000.0
#define VANE_R1	8200.0
#define VANE_R2	1000.0
#define VANE_R3	2200.0
#define VANE_R4	3900.0
#define VANE_R5	16000.0
#define VANE_R6	120000.0
#define VANE_R7	64900.0

#define RPAIR(a,b)	(1.0/(1.0/(a)+1.0/(b)))
#define RADC(a)	(4096*(a)/((a)+VANE_RBASE))

static int16_t minAdcValue;
static WindDirection_t WindDirections[] =
{
	{  0, RADC(VANE_R0), 0,                  0.000000f,  1.000000f },
	{  1, RADC(RPAIR(VANE_R0, VANE_R1)), 0,  0.382683f,  0.923880f },
	{  2, RADC(VANE_R1), 0,                  0.707107f,  0.707107f },
	{  3, RADC(RPAIR(VANE_R1, VANE_R2)), 0,  0.923880f,  0.382683f },
	{  4, RADC(VANE_R2), 0,                  1.000000f,  0.000000f },
	{  5, RADC(RPAIR(VANE_R2, VANE_R3)), 0,  0.923880f, -0.382683f },
	{  6, RADC(VANE_R3), 0,                  0.707107f, -0.707107f },
	{  7, RADC(RPAIR(VANE_R3, VANE_R4)), 0,  0.382683f, -0.923880f },
	{  8, RADC(VANE_R4), 0,                  0.000000f, -1.000000f },
	{  9, RADC(RPAIR(VANE_R4, VANE_R5)), 0, -0.382683f, -0.923880f },
	{ 10, RADC(VANE_R5), 0,                 -0.707107f, -0.707107f },
	{ 11, RADC(RPAIR(VANE_R5, VANE_R6)), 0, -0.923880f, -0.382683f },
	{ 12, RADC(VANE_R6), 0,                 -1.000000f,  0.000000f },
	{ 13, RADC(RPAIR(VANE_R6, VANE_R7)), 0, -0.923880f,  0.382683f },
	{ 14, RADC(VANE_R7), 0,                 -0.707107f,  0.707107f },
	{ 15, RADC(RPAIR(VANE_R7, VANE_R0)), 0, -0.382683f,  0.923880f },
};

void vane_init ()
{
    vane_reset ();

    for (int i=0 ; i < 16 ; i ++) {
        double r = i*M_PI*2/16;
        //float s1 = WindDirections[i].sinCoefficient;
        //float c1 = WindDirections[i].cosCoefficient;
        WindDirections[i].sinCoefficient = sin(r);
        WindDirections[i].cosCoefficient = cos(r);
        //float s2 = WindDirections[i].sinCoefficient;
        //float c2 = WindDirections[i].cosCoefficient;
        //printf_P(PSTR("%2d\tsin %8.6f -> %8.6f\tcos %8.6f -> %8.6f\n"), s1, s2, c1, c2);
    }

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

static float wind_x, wind_y;

void vane_reset ()
{
    wind_x = 0;
    wind_y = 0;
}

void vane_add (uint8_t directionIndex, uint16_t windCounter)
{
    if (directionIndex < 0 || directionIndex > 15)
        return;

    wind_x += WindDirections[directionIndex].cosCoefficient * windCounter;
    wind_y += WindDirections[directionIndex].sinCoefficient * windCounter;
}

Vane_t vane_calculate ()
{
    Vane_t vane;
    vane.direction = atan2(wind_y, wind_x);
    vane.force = hypot(wind_x, wind_y);
    return vane;
}
