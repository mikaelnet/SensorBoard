/*
 * ds1820_driver.h
 *
 * Created: 2012-05-20 13:05:45
 *  Author: mikael
 */ 


#ifndef DS1820_DRIVER_H_
#define DS1820_DRIVER_H_

#if 0

#include <avr/io.h>
#include <stdbool.h>

class DS1820
{
	private:
		PORT_t *_port;
		uint8_t _pin;
		uint8_t _pin_bm;
		
		bool w1_reset();
		uint8_t w1_bit_io (bool bit);
		uint8_t w1_byte_wr (uint8_t byte);
		uint8_t w1_byte_rd ();
		uint8_t w1_rom_search (uint8_t diff, uint8_t *id);
		void w1_command (uint8_t command, uint8_t *id);
		
	public:	
		DS1820 (PORT_t *port, uint8_t pin);
		bool startConversion ();
		
		uint16_t readTemperature (uint8_t *id);
		uint16_t readFirst ();	// replace this one
		
		float convert2temperature (uint16_t reading);
		
		// Requires an allocated buf of at least 24 bytes
		void addressToString (uint8_t *id, char *buf);
};

#endif
#endif /* DS1820_DRIVER_H_ */