/*
 * dht22_driver.c
 *
 * Created: 2012-11-27 20:55:33
 *  Author: mikael
 */ 


#include "dht22_driver.h"
#include "../core/cpu.h"

extern "C" {
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#include <avr/pgmspace.h>
	#include <util/delay.h>
	#include <util/atomic.h>
}

#define DIRECT_READ(base, mask)			(((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)	((*(base+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)	((*(base+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)	((*(base+2)) &= ~(mask))

// This should be 40, but the sensor is adding an extra bit at the start
#define DHT22_DATA_BIT_COUNT 41

DHT22::DHT22(PORT_t *port, uint8_t pin)
{
	_port = port;
	_pin_bm = 1 << pin;
	_lastReadTime = millis();
	_lastHumidity = DHT22_ERROR_VALUE;
	_lastTemperature = DHT22_ERROR_VALUE;
	
	// Requires external pull-up
	_port->DIRCLR = _pin_bm;
	_port->OUTSET = _pin_bm;
}

//
// Read the 40 bit data stream from the DHT 22
// Store the results in private member data to be read by public member functions
//
DHT22_ERROR_t DHT22::readData()
{
	//uint8_t bitmask = _bitmask;
	//volatile uint8_t *reg asm("r30") = _baseReg;
	uint8_t pin_bm = _pin_bm;
	uint8_t retryCount;
	uint8_t bitTimes[DHT22_DATA_BIT_COUNT];
	int currentHumidity;
	int currentTemperature;
	uint8_t checkSum, csPart1, csPart2, csPart3, csPart4;
	unsigned long currentTime;
	int i;

	currentHumidity = 0;
	currentTemperature = 0;
	checkSum = 0;
	currentTime = millis();
	for(i = 0; i < DHT22_DATA_BIT_COUNT; i++)
	{
		bitTimes[i] = 0;
	}

	if(currentTime - _lastReadTime < 2000)
	{
		// Caller needs to wait 2 seconds between each call to readData
		return DHT_ERROR_TOOQUICK;
	}
	_lastReadTime = currentTime;

	// Pin needs to start HIGH, wait until it is HIGH with a timeout
	_port->DIRCLR = pin_bm;
	retryCount = 0;
	do
	{
		if (retryCount > 125)
		{
			return DHT_BUS_HUNG;
		}
		retryCount++;
		_delay_us(2);
	} while(!(_port->IN & pin_bm));

	// Send the activate pulse
	_port->DIRSET = pin_bm;	
	_port->OUTCLR = pin_bm;	
	_delay_us(1100); // 1.1 ms or ~20-40ms??
	
	_port->DIRCLR = pin_bm;
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
	} while(!(_port->IN & pin_bm));
	
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
	} while((_port->IN & pin_bm));
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
		} while(!(_port->IN & pin_bm));
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
		} while((_port->IN & pin_bm));
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

	_lastHumidity = currentHumidity & 0x7FFF;
	if(currentTemperature & 0x8000)
	{
		// Below zero, non standard way of encoding negative numbers!
		// Convert to native negative format.
		_lastTemperature = -currentTemperature & 0x7FFF;
	}
	else
	{
		_lastTemperature = currentTemperature;
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

//
// This is used when the millis clock rolls over to zero
//
void DHT22::clockReset()
{
	_lastReadTime = millis();
}

#if 0

#include "dht22_driver.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdio.h>

PORT_t *_port;
//uint8_t _pin;
#define _pin_bm		_BV(4)	// Fixa så att detta blir konfigurerbart!
//uint8_t _pin_bm;
//uint8_t _pin_bm = _BV(4);
uint8_t _type;
uint8_t _data[6];

bool dht_read();

void dht_init(PORT_t *port, uint8_t pin, uint8_t type)
{
	_port = port;
	//_pin = pin;
	//_pin_bm = _BV(_pin);
	//_pin_bm = _BV(4);//1 << pin;
	_type = type;
	
	// Requires external pullup
	_port->DIRCLR = _pin_bm;
	_port->OUTSET = _pin_bm;
}


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