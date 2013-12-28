/*
 * onewire_driver.h
 *
 * Created: 2013-12-28 22:09:00
 *  Author: mikael
 */ 


#ifndef ONEWIRE_DRIVER_H_
#define ONEWIRE_DRIVER_H_

#include <avr/io.h>
#include <stdbool.h>

#define MATCH_ROM		0x55
#define SKIP_ROM		0xCC
#define	SEARCH_ROM		0xF0


class OneWire
{
	private:
		PORT_t *_port;
		uint8_t _pin;
		uint8_t _pin_bm;

		// global search state
		uint8_t ROM_NO[8];
		uint8_t LastDiscrepancy;
		uint8_t LastFamilyDiscrepancy;
		bool LastDeviceFlag;

	public:
		OneWire(PORT_t *port, uint8_t pin);

		// Perform a 1-Wire reset cycle. Returns 1 if a device responds
		// with a presence pulse.  Returns 0 if there is no device or the
		// bus is shorted or otherwise held low for more than 250uS
		bool reset(void);

		// Issue a 1-Wire rom select command, you do the reset first.
		void select( uint8_t rom[8]);

		// Issue a 1-Wire rom skip command, to address all on bus.
		void skip(void);

		// Write a byte. If 'power' is one then the wire is held high at
		// the end for parasitically powered devices. You are responsible
		// for eventually depowering it by calling depower() or doing
		// another read or write.
		void write(uint8_t v, uint8_t power = 0);

		void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0);

		// Read a byte.
		uint8_t read(void);

		void read_bytes(uint8_t *buf, uint16_t count);

		// Write a bit. The bus is always left powered at the end, see
		// note in write() about that.
		void write_bit(uint8_t v);

		// Read a bit.
		uint8_t read_bit(void);

		// Stop forcing power onto the bus. You only need to do this if
		// you used the 'power' flag to write() or used a write_bit() call
		// and aren't about to do another read or write. You would rather
		// not leave this powered if you don't have to, just in case
		// someone shorts your bus.
		void depower(void);

		// Clear the search state so that if will start from the beginning again.
		void reset_search();

		// Look for the next device. Returns 1 if a new address has been
		// returned. A zero might mean that the bus is shorted, there are
		// no devices, or you have already retrieved all of them.  It
		// might be a good idea to check the CRC to make sure you didn't
		// get garbage.  The order is deterministic. You will always get
		// the same devices in the same order.
		bool search(uint8_t *newAddr);
};


#endif /* ONEWIRE_DRIVER_H_ */