/*
 * i2c_driver.h
 *
 * Created: 2013-12-29 11:30:55
 *  Author: mikael
 */ 


#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

#include <avr/io.h>

#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)

class I2C 
{
	private:
		TWI_t *_twi;
	
	public:
		I2C (TWI_t *twi, uint8_t baudRateRegisterSetting);
	
		void beginTransmission(uint8_t address);
		void endTransmission(bool stop = true);
	
		void write (uint8_t data);
		void write (uint8_t *data, int8_t length);

		void requestFrom(uint8_t address, uint8_t quantity, bool stop = true);
		uint8_t available();
		uint8_t read();
};



#endif /* I2C_DRIVER_H_ */