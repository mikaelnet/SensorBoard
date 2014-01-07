/*
 * ds1820_tests.cpp
 *
 * Created: 2013-12-28 13:59:53
 *  Author: mikael
 */ 

#if DS1820_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

#include <util/atomic.h>

#include "../core/board.h"
#include "../drivers/ds1820_driver.h"
//#include "../drivers/onewire_driver.h"

OneWire_t oneWire;

#ifdef __cplusplus
extern "C" {
#endif

void ds1820_tests_setup()
{
	OneWire_Init(&oneWire, &PORTD, 5);
}

void ds1820_tests()
{
	uint8_t addr[8];
	//uint8_t data[12];
	//uint8_t type_s;
	//bool present;
	
	sen_enable();
	_delay_us(5);
	
	if (!DS1820_FindNext(&oneWire, addr)) {
		puts_P(PSTR("No sensors found"));
		OneWire_reset_search(&oneWire);
		//return;
	}
	
	/*
	if ( !OneWire_search(&oneWire, addr)) {
		puts_P(PSTR("No more addresses."));
		OneWire_reset_search(&oneWire);
		return;
	}*/
	
	printf_P(PSTR("ROM ="));
	for (uint8_t i=0 ; i < 8 ; i ++) {
		printf_P(PSTR(" %02X"), addr[i]);	
	}
	
	/*switch(addr[0]) {
		case 0x10:
			puts_P(PSTR("  Chip = DS18S20"));  // or old DS1820
			type_s = 1;
			break;
		case 0x28:
			puts_P(PSTR("  Chip = DS18B20"));
			type_s = 0;
			break;
		case 0x22:
			puts_P(PSTR("  Chip = DS1822"));
			type_s = 0;
			break;
		default:
			puts_P(PSTR("Device is not a DS18x20 family device."));
			return;
	}*/

	DS1820_StartConvertion(&oneWire, addr);	
	/*OneWire_reset(&oneWire);
	OneWire_select(&oneWire, addr);
	OneWire_write(&oneWire, 0x44, 1);         // start conversion, with parasite power on at the end
	*/
	
	_delay_ms(750);     // maybe 750ms is enough, maybe not
	// we might do a OneWire_depower() here, but the reset will take care of it.
	
	//if (!DS1820_ReadTemperature(&oneWire, addr, data)) {
	//	puts_P(PSTR("Error reading temperature"));
	//}

	/*present = OneWire_reset(&oneWire);
	OneWire_select(&oneWire, addr);
	OneWire_write(&oneWire, 0xBE, 0);         // Read Scratchpad

	printf_P(PSTR("  Data = %d"), present);*/
	//for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
		//data[i] = OneWire_read(&oneWire);
		//printf_P(PSTR(" %02X"), data[i]);
	//}
	
	/*uint16_t raw = (data[1] << 8) | data[0];
	if (type_s) {
		raw = raw << 3; // 9 bit resolution default
		if (data[7] == 0x10) {
			// count remain gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
		}
	} 
	else {
		uint8_t cfg = (data[4] & 0x60);
		if (cfg == 0x00) 
			raw = raw << 3;  // 9 bit resolution, 93.75 ms
		else if (cfg == 0x20) 
			raw = raw << 2; // 10 bit res, 187.5 ms
		else if (cfg == 0x40) 
			raw = raw << 1; // 11 bit res, 375 ms
		// default is 12 bit resolution, 750 ms conversion time
	}*/
	uint16_t raw = DS1820_ReadTemperature(&oneWire, addr);
	
	sen_disable();
	
	printf_P(PSTR("Temperature  %4d.%01d%cC\n"), raw >> 4, (raw << 12) / 6553, 0xB0);
}

#ifdef __cplusplus
}
#endif

#endif