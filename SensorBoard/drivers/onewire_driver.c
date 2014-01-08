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


void OneWire_Init (OneWire_t *oneWire, PORT_t *port, uint8_t pin)
{
	oneWire->port = port;
	oneWire->pin_bm = 1 << pin;

	OneWire_reset_search(oneWire);
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
//
bool OneWire_reset(OneWire_t *oneWire)
{
	PORT_t *port = oneWire->port;
	uint8_t pin_bm = oneWire->pin_bm;
	uint8_t retries = 125;
	bool result;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		port->DIRCLR = pin_bm;
		// wait until the wire is high... just in case
		do {
			if (--retries == 0) return 0;
			_delay_us(2);
		} while ( !(port->IN & pin_bm));

		port->OUTCLR = pin_bm;
		port->DIRSET = pin_bm;
	}

	_delay_us(500);	// at least 480us
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		port->DIRCLR = pin_bm;	// allow it to float
		_delay_us(80);
		result = !(port->IN & pin_bm);
	}	
	_delay_us(420);
	return result;
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
void OneWire_write_bit(OneWire_t *oneWire, uint8_t v)
{
	PORT_t *port = oneWire->port;
	uint8_t pin_bm = oneWire->pin_bm;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (v & 1) {
			port->OUTCLR = pin_bm;
			port->DIRSET = pin_bm;	// drive output low
			_delay_us(10);
			port->OUTSET = pin_bm;	// drive output high
			_delay_us(55);
		} else {
			port->OUTCLR = pin_bm;
			port->DIRSET = pin_bm;	// drive output low
			_delay_us(65);
			port->OUTSET = pin_bm;	// drive output high
			_delay_us(5);
		}
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
uint8_t OneWire_read_bit(OneWire_t *oneWire)
{
	PORT_t *port = oneWire->port;
	uint8_t pin_bm = oneWire->pin_bm;
	uint8_t result;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		port->DIRSET = pin_bm;
		port->OUTCLR = pin_bm;
		_delay_us(3);
		port->DIRCLR = pin_bm;	// let pin float, pull up will raise
		_delay_us(10);
		result = port->IN & pin_bm ? 1:0;
		_delay_us(53);
	}
	return result;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to true, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
void OneWire_write(OneWire_t *oneWire, uint8_t v, bool power) 
{
	uint8_t bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		OneWire_write_bit(oneWire, (bitMask & v)?1:0);
	}
	
	if (!power) {
		PORT_t *port = oneWire->port;
		uint8_t pin_bm = oneWire->pin_bm;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			port->DIRCLR = pin_bm;
			port->OUTCLR = pin_bm;
		}
	}
}

void OneWire_write_bytes(OneWire_t *oneWire, const uint8_t *buf, uint16_t count, bool power) 
{
	for (uint16_t i = 0 ; i < count ; i++)
		OneWire_write(oneWire, buf[i], power);
		
	if (!power) {
		PORT_t *port = oneWire->port;
		uint8_t pin_bm = oneWire->pin_bm;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			port->DIRCLR = pin_bm;
			port->OUTCLR = pin_bm;
		}
	}
}

//
// Read a byte
//
uint8_t OneWire_read(OneWire_t *oneWire) 
{
	uint8_t r = 0;

	for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
		if (OneWire_read_bit(oneWire))
			r |= bitMask;
	}
	return r;
}

void OneWire_read_bytes(OneWire_t *oneWire, uint8_t *buf, uint16_t count) 
{
	for (uint16_t i = 0 ; i < count ; i++)
		buf[i] = OneWire_read(oneWire);
}

//
// Do a ROM select
//
void OneWire_select(OneWire_t *oneWire, uint8_t rom[8])
{
	OneWire_write(oneWire, MATCH_ROM, 0);           // Choose ROM
	for(uint8_t i = 0; i < 8; i++) 
		OneWire_write(oneWire, rom[i], 0);
}

//
// Do a ROM skip
//
void OneWire_skip(OneWire_t *oneWire)
{
	OneWire_write(oneWire, SKIP_ROM, 0);           // Skip ROM
}

void OneWire_depower(OneWire_t *oneWire)
{
	oneWire->port->DIRCLR = oneWire->pin_bm;
}

//
// You need to use this function to start a search again from the beginning.
// You do not need to do it for the first search, though you could.
//
void OneWire_reset_search(OneWire_t *oneWire)
{
	// reset the search state
	oneWire->LastDiscrepancy = 0;
	oneWire->LastDeviceFlag = false;
	oneWire->LastFamilyDiscrepancy = 0;
	for(int i = 7; ; i--)
	{
		oneWire->ROM_NO[i] = 0;
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
bool OneWire_search(OneWire_t *oneWire, uint8_t *newAddr)
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
	if (!oneWire->LastDeviceFlag)
	{
		// 1-Wire reset
		if (!OneWire_reset(oneWire))
		{
			// reset the search
			oneWire->LastDiscrepancy = 0;
			oneWire->LastDeviceFlag = false;
			oneWire->LastFamilyDiscrepancy = 0;
			return false;
		}

		// issue the search command
		OneWire_write(oneWire, SEARCH_ROM, 0);

		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = OneWire_read_bit(oneWire);
			cmp_id_bit = OneWire_read_bit(oneWire);

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
					if (id_bit_number < oneWire->LastDiscrepancy)
						search_direction = ((oneWire->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == oneWire->LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
							oneWire->LastFamilyDiscrepancy = last_zero;
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					oneWire->ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					oneWire->ROM_NO[rom_byte_number] &= ~rom_byte_mask;

				// serial number search direction write bit
				OneWire_write_bit(oneWire, search_direction);

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
			oneWire->LastDiscrepancy = last_zero;

			// check for last device
			if (oneWire->LastDiscrepancy == 0)
				oneWire->LastDeviceFlag = true;

			search_result = true;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !oneWire->ROM_NO[0])
	{
		oneWire->LastDiscrepancy = 0;
		oneWire->LastDeviceFlag = false;
		oneWire->LastFamilyDiscrepancy = 0;
		search_result = false;
	}
	for (int i = 0; i < 8; i++) 
		newAddr[i] = oneWire->ROM_NO[i];
		
	return search_result;
}

#endif