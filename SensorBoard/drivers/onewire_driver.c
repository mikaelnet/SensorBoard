/*
 * onewire_driver.cpp
 *
 * Created: 2013-12-28 22:08:40
 *  Author: mikael
 */ 

#if DS1820_ENABLE==1

#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "onewire_driver.h"

static PORT_t *_port;
static uint8_t _pin;
static uint8_t _pin_bm;

// global search state
static uint8_t ROM_NO[8];
static uint8_t LastDiscrepancy;
static uint8_t LastFamilyDiscrepancy;
static bool LastDeviceFlag;


void OneWire_begin (PORT_t *port, uint8_t pin)
{
	_port = port;
	_pin = pin;
	_pin_bm = 1 << pin;

	OneWire_reset_search();
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
//
bool OneWire_reset(void)
{
	bool result;
	uint8_t retries = 125;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRCLR = _pin_bm;
		// wait until the wire is high... just in case
		do {
			if (--retries == 0) return 0;
			_delay_us(2);
		} while ( !(_port->IN & _pin_bm));

		_port->OUTCLR = _pin_bm;
		_port->DIRSET = _pin_bm;
	}

	_delay_us(500);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRCLR = _pin_bm;	// allow it to float
		_delay_us(80);
		result = !(_port->IN & _pin_bm);
	}	
	_delay_us(420);
	return result;
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
void OneWire_write_bit(uint8_t v)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (v & 1) {
			_port->OUTCLR = _pin_bm;
			_port->DIRSET = _pin_bm;	// drive output low
			_delay_us(10);
			_port->OUTSET = _pin_bm;	// drive output high
			_delay_us(55);
		} else {
			_port->OUTCLR = _pin_bm;
			_port->DIRSET = _pin_bm;	// drive output low
			_delay_us(65);
			_port->OUTSET = _pin_bm;	// drive output high
			_delay_us(5);
		}
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
uint8_t OneWire_read_bit(void)
{
	uint8_t r;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		_port->DIRSET = _pin_bm;
		_port->OUTCLR = _pin_bm;
		_delay_us(3);
		_port->DIRCLR = _pin_bm;	// let pin float, pull up will raise
		_delay_us(10);
		r = _port->IN & _pin_bm ? 1:0;
		_delay_us(53);
	}
	return r;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
void OneWire_write(uint8_t v, uint8_t power /* = 0 */) {
	uint8_t bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		OneWire_write_bit( (bitMask & v)?1:0);
	}
	
	if ( !power) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			_port->DIRCLR = _pin_bm;
			_port->OUTCLR = _pin_bm;
		}
	}
}

void OneWire_write_bytes(const uint8_t *buf, uint16_t count, bool power /* = 0 */) {
	for (uint16_t i = 0 ; i < count ; i++)
		OneWire_write(buf[i], power);
		
	if (!power) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			_port->DIRCLR = _pin_bm;
			_port->OUTCLR = _pin_bm;
		}
	}
}

//
// Read a byte
//
uint8_t OneWire_read() {
	uint8_t r = 0;

	for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
		if (OneWire_read_bit())
			r |= bitMask;
	}
	return r;
}

void OneWire_read_bytes(uint8_t *buf, uint16_t count) {
	for (uint16_t i = 0 ; i < count ; i++)
		buf[i] = OneWire_read();
}

//
// Do a ROM select
//
void OneWire_select(uint8_t rom[8])
{
	OneWire_write(MATCH_ROM, 0);           // Choose ROM
	for(uint8_t i = 0; i < 8; i++) 
		OneWire_write(rom[i], 0);
}

//
// Do a ROM skip
//
void OneWire_skip()
{
	OneWire_write(SKIP_ROM, 0);           // Skip ROM
}

void OneWire_depower()
{
	_port->DIRCLR = _pin_bm;
}

//
// You need to use this function to start a search again from the beginning.
// You do not need to do it for the first search, though you could.
//
void OneWire_reset_search()
{
	// reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = false;
	LastFamilyDiscrepancy = 0;
	for(int i = 7; ; i--)
	{
		ROM_NO[i] = 0;
		if (i == 0)
			break;
	}
}

//
// Perform a search. If this function returns a '1' then it has
// enumerated the next device and you may retrieve the ROM from the
// OneWire_address variable. If there are no devices, no further
// devices, or something horrible happens in the middle of the
// enumeration then a 0 is returned.  If a new device is found then
// its address is copied to newAddr.  Use OneWire_reset_search() to
// start over.
//
// --- Replaced by the one from the Dallas Semiconductor web site ---
//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//
bool OneWire_search(uint8_t *newAddr)
{
	// initialize for search
	uint8_t id_bit_number = 1;
	uint8_t last_zero = 0;
	uint8_t rom_byte_number = 0;
	uint8_t rom_byte_mask = 1;
	bool search_result = false;
	
	uint8_t id_bit, cmp_id_bit;

	unsigned char search_direction;

	// if the last call was not the last one
	if (!LastDeviceFlag)
	{
		// 1-Wire reset
		if (!OneWire_reset())
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = false;
			LastFamilyDiscrepancy = 0;
			return false;
		}

		// issue the search command
		OneWire_write(SEARCH_ROM, 0);

		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = OneWire_read_bit();
			cmp_id_bit = OneWire_read_bit();

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
				break;
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
					search_direction = id_bit;  // bit write value for search
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
						search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
							LastFamilyDiscrepancy = last_zero;
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;

				// serial number search direction write bit
				OneWire_write_bit(search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number ++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number ++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!(id_bit_number < 65))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
				LastDeviceFlag = true;

			search_result = true;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = false;
		LastFamilyDiscrepancy = 0;
		search_result = false;
	}
	for (int i = 0; i < 8; i++) 
		newAddr[i] = ROM_NO[i];
		
	return search_result;
}

#endif