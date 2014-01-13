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

#if DHT22_ENABLE==1

// This should be 40, but the sensor is adding an extra bit at the start
#define DHT22_DATA_BIT_COUNT 41

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
	
	// Requires external pull-up
	port->DIRCLR = pin_bm;
	port->OUTSET = pin_bm;
}

//
// Read the 40 bit data stream from the DHT 22
// Store the results in private member data to be read by public member functions
//
DHT22_ERROR_t DHT22_readData(DHT22_t *dht22)
{
	uint8_t pin_bm = dht22->pin_bm;
	PORT_t *port = dht22->port;
	uint8_t retryCount;
	uint8_t bitTimes[DHT22_DATA_BIT_COUNT];
	int currentHumidity;
	int currentTemperature;
	uint8_t checkSum, csPart1, csPart2, csPart3, csPart4;
	int i;

	currentHumidity = 0;
	currentTemperature = 0;
	checkSum = 0;
	//currentTime = millis();
	for(i = 0; i < DHT22_DATA_BIT_COUNT; i++)
	{
		bitTimes[i] = 0;
	}

	/*if(currentTime - _lastReadTime < 2000)
	{
		// Caller needs to wait 2 seconds between each call to readData
		return DHT_ERROR_TOOQUICK;
	}
	_lastReadTime = currentTime;*/

	// Pin needs to start HIGH, wait until it is HIGH with a timeout
	port->DIRCLR = pin_bm;
	retryCount = 0;
	do
	{
		if (retryCount > 125)
		{
			return DHT_BUS_HUNG;
		}
		retryCount++;
		_delay_us(2);
	} while(!(port->IN & pin_bm));

	// Send the activate pulse
	port->DIRSET = pin_bm;	
	port->OUTCLR = pin_bm;	
	_delay_us(1100); // 1.1 ms or ~20-40ms??
	
	port->DIRCLR = pin_bm;
	// Find the start of the ACK Pulse
	retryCount = 0;
	do
	{
		if (retryCount > 25) //(Spec is 20 to 40 us, 25*2 == 50 us)
		{
			return DHT_ERROR_NOT_PRESENT;
		}
		retryCount++;
		_delay_us(2);
	} while(!(port->IN & pin_bm));
	
	// Find the end of the ACK Pulse
	retryCount = 0;
	do
	{
		if (retryCount > 50) //(Spec is 80 us, 50*2 == 100 us)
		{
			return DHT_ERROR_ACK_TOO_LONG;
		}
		retryCount++;
		_delay_us(2);
	} while((port->IN & pin_bm));
	// Read the 40 bit data stream
	for(i = 0; i < DHT22_DATA_BIT_COUNT; i++)
	{
		// Find the start of the sync pulse
		retryCount = 0;
		do
		{
			if (retryCount > 35) //(Spec is 50 us, 35*2 == 70 us)
			{
				return DHT_ERROR_SYNC_TIMEOUT;
			}
			retryCount++;
			_delay_us(2);
		} while(!(port->IN & pin_bm));
		// Measure the width of the data pulse
		retryCount = 0;
		do
		{
			if (retryCount > 50) //(Spec is 80 us, 50*2 == 100 us)
			{
				return DHT_ERROR_DATA_TIMEOUT;
			}
			retryCount++;
			_delay_us(2);
		} while((port->IN & pin_bm));
		bitTimes[i] = retryCount;
	}
	// Now bitTimes have the number of retries (us *2)
	// that were needed to find the end of each data bit
	// Spec: 0 is 26 to 28 us
	// Spec: 1 is 70 us
	// bitTimes[x] <= 11 is a 0
	// bitTimes[x] >  11 is a 1
	// Note: the bits are offset by one from the data sheet, not sure why
	for(i = 0; i < 16; i++)
	{
		if(bitTimes[i + 1] > 11)
		{
			currentHumidity |= (1 << (15 - i));
		}
	}
	for(i = 0; i < 16; i++)
	{
		if(bitTimes[i + 17] > 11)
		{
			currentTemperature |= (1 << (15 - i));
		}
	}
	for(i = 0; i < 8; i++)
	{
		if(bitTimes[i + 33] > 11)
		{
			checkSum |= (1 << (7 - i));
		}
	}

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

	csPart1 = currentHumidity >> 8;
	csPart2 = currentHumidity & 0xFF;
	csPart3 = currentTemperature >> 8;
	csPart4 = currentTemperature & 0xFF;
	if(checkSum == ((csPart1 + csPart2 + csPart3 + csPart4) & 0xFF))
	{
		return DHT_ERROR_NONE;
	}
	return DHT_ERROR_CHECKSUM;
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

#if 0


float dht_readTemperature() {
	float f;

	if (dht_read()) {
		switch (_type) {
			case DHT11:
				f = _data[2];
				return f;

			case DHT22:
			case DHT21:
				f = _data[2] & 0x7F;
				f *= 256;
				f += _data[3];
				f /= 10;
				if (_data[2] & 0x80)
					f *= -1;
				return f;
		}
	}
	puts_P(PSTR("Read temp failed"));
	return NAN;
}

/*! \brief Reads relative humidity
 *
 *  This method can only be called once every two seconds.
 *
 *  \retval           RH%, where value need division by 10 for DHT22/21
 */
uint16_t dht_readHumidity() {
	if (dht_read()) {
		switch (_type) {
			case DHT11:
				return _data[0];
	  
			case DHT22:
			case DHT21:
				return (_data[0] << 8) | _data[1];
		}
	}
	puts_P(PSTR("Read humidity failed"));
	return NAN;
}

#include <alloca.h>

bool dht_read(void) {
	uint8_t laststate = _pin_bm;	// HIGH
	uint8_t counter = 0;
	uint8_t j = 0, i;
	register uint8_t pin_bm = _pin_bm;

	// pull the pin high and wait 250 milliseconds
	_port->DIRCLR = pin_bm;
	_delay_ms(250);	// ideal 250ms

	// Clear buffer
	_data[0] = 0;
	_data[1] = 0;
	_data[2] = 0;
	_data[3] = 0;
	_data[4] = 0;
	_data[5] = 0;
  
	uint8_t *counters = alloca(sizeof(uint8_t)*MAXTIMINGS);
	uint8_t *counterPtr = counters;

	// now pull it low for ~20 milliseconds
	_port->DIRSET = pin_bm;	// pinMode(_pin, OUTPUT);
	_port->OUTCLR = pin_bm;	// digitalWrite(_pin, LOW);
	_delay_ms(18);	// ideal 20ms. 18 works on 2MHz and _pin_bm as #define

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->OUTSET = pin_bm;	// digitalWrite(_pin, HIGH);
		_delay_us(30);	// ideal 40us. 30 works on 2MHz and _pin_bm as #define
		_port->DIRCLR = pin_bm;	// pinMode(_pin, INPUT);

		// read in timings
		for ( i = 0 ; i < MAXTIMINGS ; i ++ ) {
			counter = 0;
			// Skriv om denna med bit_is_clear / bit_is_set kanske?
			while ( (_port->IN & pin_bm) == laststate) {
				counter ++;
				//_delay_us (1);	// ideal 1us. commented out works on 2MHz and _pin_bm as #define
				if (counter == 255) {
					break;
				}
			}
			laststate = laststate == 0 ? pin_bm : 0; //(_port->IN & _pin_bm);

			*counterPtr = counter;
			counterPtr ++;

			if (counter == 255) {
				break;
			}				

			// ignore first 3 transitions
			if ((i >= 4) && (i % 2 == 0)) {
				// shove each bit into the storage bytes
				_data[j/8] <<= 1;
				if (counter > 2)
					_data[j/8] |= 1;
				j ++;
			}
		}
	}
	
	puts_P(PSTR("Raw data:"));
	for (i=0 ; i < MAXTIMINGS ; i ++) {
		printf_P(PSTR("%2d%c"), counters[i], (i % 20) == 19 ? '\n' : ' ');
	}
  
	printf_P(PSTR("\nData:"));
	for (i=0 ; i < 5 ; i++)
		printf_P(PSTR(" %02X"), _data[i]);
	puts_P(PSTR("."));  
  
	printf_P(PSTR("%d bits, checksum %02X  "), j, _data[4]);
	printf_P(PSTR("sum %02X\n"), (_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF);
  
	// check we read 40 bits and that the checksum matches
	if ((j >= 40) && (_data[4] == ((_data[0] + _data[1] + _data[2] + _data[3]) & 0xFF)) ) {
		return true;
	}
	return false;
}

#endif

#endif // DHT22_ENABLE==1
