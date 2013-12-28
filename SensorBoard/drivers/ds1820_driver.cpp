/*
 * ds1820_driver.c
 *
 * Created: 2012-05-20 13:05:28
 *  Author: mikael
 */ 

#if 0
#include "ds1820_driver.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>


#define MATCH_ROM		0x55
#define SKIP_ROM		0xCC
#define	SEARCH_ROM		0xF0



#define READ_ROM		0x33		// Only when there's only one slave
#define ALARM_SEARCH	0xEC

#define CONVERT_T		0x44		// DS1820 commands
#define READ			0xBE
#define WRITE			0x4E
#define EE_WRITE		0x48
#define EE_RECALL		0xB8

#define	SEARCH_FIRST	0xFF		// start new search
#define	PRESENCE_ERR	0xFF
#define	DATA_ERR		0xFE
#define LAST_DEVICE		0x00		// last device found
//			0x01 ... 0x40: continue searching

DS1820::DS1820 (PORT_t *port, uint8_t pin)
{
	_port = port;
	_pin = pin;
	_pin_bm = 1 << pin;
}	

bool DS1820::startConversion()
{
	if (bit_is_set(_port->IN, _pin)) {
		w1_command (CONVERT_T, NULL);
		_port->OUTSET = _pin_bm;
		_port->DIRSET = _pin_bm;  // parasite power on
		return true;
	}
	return false;
}

bool DS1820::w1_reset()
{
	bool err;
	_port->OUTCLR = _pin_bm;
	_port->DIRSET = _pin_bm;
	
	_delay_us(500);	// 480
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRCLR = _pin_bm;
		_delay_us(80);	// 66
		err = bit_is_set(_port->IN,  _pin);
	}
	
	_delay_us(420);	// 480-66
	if (bit_is_clear(_port->IN, _pin))
	err = true;
	
	return err;
}

uint8_t DS1820::w1_bit_io (bool bit)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRSET = _pin_bm;
		_delay_us(3);	// 1 us
		if (bit)
			_port->DIRCLR = _pin_bm;
		_delay_us(10);	// 15-1 us
  
		if (bit_is_clear(_port->IN, _pin))
			bit = 0;
		
		_delay_us(55);	// 60-15 us
		_port->DIRCLR = _pin_bm;
	}
	
	return bit;
}

uint8_t DS1820::w1_byte_wr (uint8_t byte)
{
	uint8_t i = 8, j;
	do {
		j = w1_bit_io (byte & 1);
		byte >>= 1;
		if (j)
			byte |= 0x80;
	} while ( --i );
	
	return byte;
}

inline uint8_t DS1820::w1_byte_rd ()
{
	return w1_byte_wr (0xFF);
}

uint8_t DS1820::w1_rom_search (uint8_t diff, uint8_t *id)
{
	if (w1_reset())
		return PRESENCE_ERR;			// error, no device found

	w1_byte_wr (SEARCH_ROM);			// ROM search command
	uint8_t next_diff = LAST_DEVICE;	// unchanged on last device

	uint16_t i = 8 * 8;					// 8 bytes
	do {
		uint8_t j = 8;					// 8 bits
		do {
			bool b = w1_bit_io (1);		// read bit
			if (w1_bit_io (1)) {		// read complement bit
				if (b)					// 11
				{
					printf_P(PSTR("i=%d, j=%d, diff=%d\n"), i, j, diff);
					return DATA_ERR;	// data error
				}					
			}
			else {
				if (!b) {				// 00 = 2 devices
					if (diff > i || ((*id & 1) && diff != i) ) {
						b = 1;			// now 1
						next_diff = i;	// next pass 0
					}
				}
			}
			
			w1_bit_io (b);     			// write bit
			*id >>= 1;
			if (b)						// store bit
				*id |= 0x80;
			i --;
		} while ( --j );
		id ++;							// next byte
	} while (i);

	return next_diff;					// to continue search
}

void DS1820::w1_command (uint8_t command, uint8_t *id)
{
	w1_reset();
	
	if (id) {
		w1_byte_wr (MATCH_ROM);		// to a single device
		
		uint8_t i = 8;
		do {
			w1_byte_wr (*id);
			id ++;
		} while (--i);
	}
	else {
		w1_byte_wr (SKIP_ROM);		// to all devices
	}
	
	w1_byte_wr (command);
}


uint16_t DS1820::readTemperature (uint8_t *id)
{
	uint16_t temperature;
	w1_command(READ, id);
	temperature = w1_byte_rd();		// low byte
	temperature |= w1_byte_rd() << 8;	// high byte
	if (id[0] == 0x10)			// 9 -> 12 bit
		temperature <<= 3;
		
	return temperature;
}

uint16_t DS1820::readFirst ()
{
	uint8_t id[8];
	uint16_t temperature;

	for (uint8_t diff = SEARCH_FIRST ; diff != LAST_DEVICE ; ) {
		diff = w1_rom_search (diff, id);

		if (diff == PRESENCE_ERR) {
			puts_P(PSTR("No Sensor found"));
			break;
		}
		
		if (diff == DATA_ERR) {
			puts_P(PSTR("Bus Error"));
			break;
		}
		
		if (id[0] == 0x28 || id[0] == 0x10) {	// temperature sensor
			w1_byte_wr (READ);			// read command
			temperature = w1_byte_rd();		// low byte
			temperature |= w1_byte_rd() << 8;	// high byte

			if (id[0] == 0x10)			// 9 -> 12 bit
				temperature <<= 3;
	
			printf_P(PSTR("%04X  %4d.%01d%cC\n"), temperature, temperature >> 4, (temperature << 12) / 6553, 0xB0);
			
			return temperature;
		}
	}
	return 0;
}

float DS1820::convert2temperature (uint16_t reading)
{
	return reading / 16.0;
}

const char *hex = "0123456789ABCDEF";
void DS1820::addressToString (uint8_t *id, char *buf)
{
	char *ptr = buf;
	for (uint8_t i=0 ; i < 8 ; i++)
	{
		*ptr++ = hex[*id >> 4];
		*ptr++ = hex[*id & 0x0F];
		*ptr++ = ':';
		id ++;
	}
	// NULL-terminate last char
	ptr--;
	*ptr = 0;
}

#endif