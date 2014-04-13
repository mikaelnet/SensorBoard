/*
 * dht22_driver.c
 *
 * Created: 2012-11-27 20:55:33
 *  Author: mikael
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "dht22_driver.h"
#include "../core/cpu.h"

#include <avr/pgmspace.h>
#include <stdio.h>

#if DHT22_ENABLE==1

// This should be 40, but the sensor is adding an extra bit at the start
#define DHT22_DATA_BIT_COUNT 40

/*static PORT_t *_port;
static uint8_t _pin_bm;
static short int _lastHumidity;
static short int _lastTemperature;
*/

void DHT22_init(DHT22_t *dht22, PORT_t *port, uint8_t pin)
{
	uint8_t pin_bm = 1 << pin;
	dht22->port = port;
	dht22->pin_bm = pin_bm;
	dht22->lastHumidity = DHT22_ERROR_VALUE;
	dht22->lastTemperature = DHT22_ERROR_VALUE;
    dht22->error = DHT_ERROR_NONE;
    dht22->lastMeasure = cpu_second();

	// Requires external pull-up
	port->DIRCLR = pin_bm;
	port->OUTSET = pin_bm;
}

inline uint16_t DHT22_waitForHigh (PORT_t *port, uint8_t pin_bm, uint8_t timeout)
{
    register uint16_t startTime, pulseTime = 0;
    startTime = cpu_microsecond();
    while (!(port->IN & pin_bm)) {
        pulseTime = cpu_microsecond() - startTime;
        if (pulseTime > timeout)
            return -1;
    }
    return pulseTime;
}

inline int16_t DHT22_waitForLow (PORT_t *port, uint8_t pin_bm, uint8_t timeout)
{
    register uint16_t startTime, pulseTime = 0;
    startTime = cpu_microsecond();
    while (port->IN & pin_bm) {
        pulseTime = cpu_microsecond() - startTime;
        if (pulseTime > timeout)
            return -1;
    }
    return pulseTime;
}

/*
    Power must be turned on at least 1 second before calling this method
    This method can only be called once every 2 second.
*/
DHT22_ERROR_t DHT22_readData(DHT22_t *dht22)
{
    uint8_t pin_bm = dht22->pin_bm;
    PORT_t *port = dht22->port;

    // Ensure we're not measuring too fast
    uint16_t measureTime = cpu_second();
    if (measureTime - dht22->lastMeasure < 2)
    {
        dht22->error = DHT_ERROR_TOOQUICK;
        return DHT_ERROR_TOOQUICK;
    }

    // Ensure the bus is idle (high)
    if (DHT22_waitForHigh(port, pin_bm, 250) < 0) {
        dht22->error = DHT_BUS_HUNG;
        return DHT_BUS_HUNG;
    }

    // Host send start signal (low .5-1ms)
    port->DIRSET = pin_bm;
    port->OUTCLR = pin_bm;
    _delay_us(1000);
    // Host pull up 20-40 us. Let external resistor do the pulling
    port->DIRCLR = pin_bm;
    port->OUTSET = pin_bm;
    _delay_us(2);
    if (DHT22_waitForLow (port, pin_bm, 80) < 0) {
        dht22->error = DHT_ERROR_NOT_PRESENT;
        puts_P(PSTR("DHT22 didn't pull low for ack"));
        return DHT_ERROR_NOT_PRESENT;
    }

    // Wait for sensor to pull low for 80us
    if (DHT22_waitForHigh (port, pin_bm, 120) < 0) {
        dht22->error = DHT_ERROR_NOT_PRESENT;
        puts_P(PSTR("DHT22 didn't pull high for ack"));
        return DHT_ERROR_NOT_PRESENT;
    }

    // Wait for sensor to pull high for 80us (ack)
    if (DHT22_waitForLow (port, pin_bm, 120) < 0) {
        dht22->error = DHT_ERROR_ACK_TOO_LONG;
        return DHT_ERROR_ACK_TOO_LONG;
    }

    uint8_t i;
    uint8_t bitTimes[DHT22_DATA_BIT_COUNT];
    for(i = 0; i < DHT22_DATA_BIT_COUNT; i++) {
        // Find the start of the sync pulse (~50us)
        if (DHT22_waitForHigh (port, pin_bm, 80) < 0) {
            dht22->error = DHT_ERROR_SYNC_TIMEOUT;
            return DHT_ERROR_SYNC_TIMEOUT;
        }
        // Measure the width of the data pulse. 26-28us = 0, 70us = 1
        uint8_t time = DHT22_waitForLow(port, pin_bm, 100);
        if (time < 0) {
            dht22->error = DHT_ERROR_DATA_TIMEOUT;
            return DHT_ERROR_DATA_TIMEOUT;
        }
        bitTimes[i] = time;
    }
    dht22->lastMeasure = cpu_second();

    int currentHumidity = 0;
    int currentTemperature = 0;
    uint8_t checkSum = 0;
    uint8_t *bitTimesPtr = bitTimes;

    for (i = 0 ; i < 16 ; i ++, bitTimesPtr ++) {
        //printf_P(PSTR(" %02X"), *bitTimesPtr);
        if (*bitTimesPtr > (28+70)/2) {
            currentHumidity  |= (1 << (15-i));
        }
    }
    //printf_P(PSTR("\nTemp:"));
    for (i = 0 ; i < 16 ; i ++, bitTimesPtr ++) {
        //printf_P(PSTR(" %02X"), *bitTimesPtr);
        if (*bitTimesPtr > (28+70)/2) {
            currentTemperature  |= (1 << (15-i));
        }
    }
    //printf_P(PSTR("\nChk:"));
    for (i = 0 ; i < 8 ; i ++, bitTimesPtr ++) {
        //printf_P(PSTR(" %02X"), *bitTimesPtr);
        if (*bitTimesPtr > (28+70)/2) {
            checkSum  |= (1 << (7-i));
        }
    }
    //printf_P(PSTR("\n"));

    dht22->lastHumidity = currentHumidity & 0x7FFF;
    if(currentTemperature & 0x8000)
    {
        // Below zero, non standard way of encoding negative numbers!
        // Convert to native negative format.
        dht22->lastTemperature = -currentTemperature & 0x7FFF;
    }
    else
    {
        dht22->lastTemperature = currentTemperature;
    }

    uint8_t csPart1 = currentHumidity >> 8;
    uint8_t csPart2 = currentHumidity & 0xFF;
    uint8_t csPart3 = currentTemperature >> 8;
    uint8_t csPart4 = currentTemperature & 0xFF;
    if(checkSum != ((csPart1 + csPart2 + csPart3 + csPart4) & 0xFF))
    {
        dht22->error = DHT_ERROR_CHECKSUM;
        return DHT_ERROR_CHECKSUM;
    }
    return DHT_ERROR_NONE;
}


// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double DHT22_dewPoint(double celsius, double humidity)
{
	// (1) Saturation Vapor Pressure = ESGG(T)
	double RATIO = 373.15 / (273.15 + celsius);
	double RHS = -7.90298 * (RATIO - 1);
	RHS += 5.02808 * log10(RATIO);
	RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
	RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
	RHS += log10(1013.246);

	// factor -3 is to adjust units - Vapor Pressure SVP * humidity
	double VP = pow(10, RHS - 3) * humidity;

	// (2) DEWPOINT = F(Vapor Pressure)
	double T = log(VP/0.61078);   // temp var
	return (241.88 * T) / (17.558 - T);
}

#endif // DHT22_ENABLE==1
