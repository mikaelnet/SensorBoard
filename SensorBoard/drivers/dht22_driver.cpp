/*
 * dht22_driver.c
 *
 * Created: 2012-11-27 20:55:33
 *  Author: mikael
 */ 

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
