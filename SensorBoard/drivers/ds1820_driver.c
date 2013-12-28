/*
 * ds1820_driver.c
 *
 * Created: 2012-05-20 13:05:28
 *  Author: mikael
 */ 

#include "ds1820_driver.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

// TODO: Fixa så att detta blir konfigurerbart!
#define W1_PORT		PORTD
#define W1_PIN		5

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

PORT_t *_port;
uint8_t _pin;
uint8_t _pin_bm;


static bool w1_reset()
{
	bool err;
	_port->OUTCLR = _pin_bm;
	_port->DIRSET = _pin_bm;
	
	_delay_us(480);
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRCLR = _pin_bm;
		_delay_us(66);
		err = bit_is_set(_port->IN,  _pin);
	}
	
	_delay_us(480-66);
	if (bit_is_clear(_port->IN, _pin))
		err = true;
		
	return err;
}

static uint8_t w1_bit_io (bool bit)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRSET = _pin_bm;
		_delay_us(1);
		if (bit)
			_port->DIRCLR = _pin_bm;
		_delay_us(15-1);
  
		if (bit_is_clear(_port->IN, _pin))
			bit = 0;
		
		_delay_us(60-15);
		_port->DIRCLR = _pin_bm;
	}
	
	return bit;
}


static uint8_t w1_byte_wr (uint8_t byte)
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


static uint8_t w1_byte_rd ()
{
	return w1_byte_wr (0xFF);
}

static uint8_t w1_rom_search (uint8_t diff, uint8_t *id)
{
	if (w1_reset())
		return PRESENCE_ERR;			// error, no device found

	w1_byte_wr (SEARCH_ROM);			// ROM search command
	uint8_t next_diff = LAST_DEVICE;	// unchanged on last device

	uint16_t i = 8 * 8;					// 8 bytes
	do {
		uint8_t j = 8;					// 8 bits
		do {
			bool b = w1_bit_io (1);	// read bit
			if (w1_bit_io (1)) {		// read complement bit
				if (b)					// 11
					return DATA_ERR;	// data error
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

static void w1_command (uint8_t command, uint8_t *id )
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

bool ds1820_termometer_init (PORT_t *port, uint8_t pin) 
{
	_port = port;
	_pin = pin;
	_pin_bm = 1 << pin;
	
	if (bit_is_set(_port->IN, _pin)) {
		w1_command (CONVERT_T, NULL);
		_port->OUTSET = _pin_bm;
		_port->DIRSET = _pin_bm;  // parasite power on
		return true;
	}
	return false;
}

uint16_t ds1820_termometer_read ()
{
	uint8_t id[8];
	uint16_t temp;

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
			temp = w1_byte_rd();		// low byte
			temp |= w1_byte_rd() << 8;	// high byte

			if (id[0] == 0x10)			// 9 -> 12 bit
				temp <<= 3;
	
			/*printf_P(PSTR("ID: "));
			for (uint8_t i = 0 ; i < 8 ; i++) {
				printf_P(PSTR("%02X"), id[i]);
			}*/
		
			printf_P(PSTR("%04X  %4d.%01d%cC\n"), temp, temp >> 4, (temp << 12) / 6553, 0xB0);
			
			return temp;
		}
	}
	return 0;
}

float ds1820_to_temperature (uint16_t temperature)
{
	return temperature / 16.0;
}

