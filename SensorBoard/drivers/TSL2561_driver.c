/*
 * TSL2561_driver.c
 *
 * Created: 2014-01-09 10:16:16
 *  Author: mikael.hogberg
 */

#include <avr/io.h>
#include <util/delay.h>

#include "TSL2561_driver.h"
#include "twi_master_driver.h"

void TSL2561_init(TSL2561_t *tsl, TWI_Master_t *twi, uint8_t addr, TSL2561IntegrationTime_t integration, TSL2561Gain_t gain)
{
	tsl->twi = twi;
	tsl->addr = addr;

	tsl->initialized = false;
	tsl->integration = integration;
	tsl->gain = gain;
}

bool TSL2561_begin(TSL2561_t *tsl) {
	uint8_t buf[1];
	buf[0] = TSL2561_REGISTER_ID;
	TWI_MasterWriteRead(tsl->twi, tsl->addr, buf, 1, 1);
	TWI_MasterWait(tsl->twi);
	uint8_t x = tsl->twi->readData[0];
	if (x & 0x0A) {
		//Serial.println("Found TSL2561");
	}
	else {
		return false;
	}
	tsl->initialized = true;

	// Set default integration time and gain
	TSL2561_setTiming(tsl, tsl->integration);
	TSL2561_setGain(tsl, tsl->gain);
	// Note: by default, the device is in power down mode on boot up
	TSL2561_disable(tsl);

	return true;
}

void TSL2561_enable(TSL2561_t *tsl)
{
	if (!tsl->initialized)
		TSL2561_begin(tsl);

	// Enable the device by setting the control bit to 0x03
	TSL2561_write8(tsl, TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
}

void TSL2561_disable(TSL2561_t *tsl)
{
	if (!tsl->initialized)
		TSL2561_begin(tsl);

	// Disable the device by setting the control bit to 0x03
	TSL2561_write8(tsl, TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWEROFF);
}


void TSL2561_setGain(TSL2561_t *tsl, TSL2561Gain_t gain) {
	if (!tsl->initialized)
		TSL2561_begin(tsl);

	TSL2561_enable(tsl);
	tsl->gain = gain;
	TSL2561_write8(tsl, TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, tsl->integration | gain);
	TSL2561_disable(tsl);
}

void TSL2561_setTiming(TSL2561_t *tsl, TSL2561IntegrationTime_t integration)
{
	if (!tsl->initialized)
		TSL2561_begin(tsl);

	TSL2561_enable(tsl);
	tsl->integration = integration;
	TSL2561_write8(tsl, TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, integration | tsl->gain);
	TSL2561_disable(tsl);
}

uint32_t TSL2561_calculateLux(TSL2561_t *tsl, uint16_t ch0, uint16_t ch1)
{
	unsigned long chScale;
	unsigned long channel1;
	unsigned long channel0;

	switch (tsl->integration)
	{
		case TSL2561_INTEGRATIONTIME_13MS:
			chScale = TSL2561_LUX_CHSCALE_TINT0;
			break;
		case TSL2561_INTEGRATIONTIME_101MS:
			chScale = TSL2561_LUX_CHSCALE_TINT1;
			break;
		default: // No scaling ... integration time = 402ms
			chScale = (1 << TSL2561_LUX_CHSCALE);
			break;
	}

	// Scale for gain (1x or 16x)
	if (!tsl->gain)
		chScale = chScale << 4;

	// scale the channel values
	channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
	channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;

	// find the ratio of the channel values (Channel1/Channel0)
	unsigned long ratio1 = 0;
	if (channel0 != 0)
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;

	// round the ratio value
	unsigned long ratio = (ratio1 + 1) >> 1;
	unsigned int b, m;

#ifdef TSL2561_PACKAGE_CS
	if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1C))
	{b=TSL2561_LUX_B1C; m=TSL2561_LUX_M1C;}
	else if (ratio <= TSL2561_LUX_K2C)
	{b=TSL2561_LUX_B2C; m=TSL2561_LUX_M2C;}
	else if (ratio <= TSL2561_LUX_K3C)
	{b=TSL2561_LUX_B3C; m=TSL2561_LUX_M3C;}
	else if (ratio <= TSL2561_LUX_K4C)
	{b=TSL2561_LUX_B4C; m=TSL2561_LUX_M4C;}
	else if (ratio <= TSL2561_LUX_K5C)
	{b=TSL2561_LUX_B5C; m=TSL2561_LUX_M5C;}
	else if (ratio <= TSL2561_LUX_K6C)
	{b=TSL2561_LUX_B6C; m=TSL2561_LUX_M6C;}
	else if (ratio <= TSL2561_LUX_K7C)
	{b=TSL2561_LUX_B7C; m=TSL2561_LUX_M7C;}
	else if (ratio > TSL2561_LUX_K8C)
	{b=TSL2561_LUX_B8C; m=TSL2561_LUX_M8C;}
#else
	if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
	{b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T;}
	else if (ratio <= TSL2561_LUX_K2T)
	{b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T;}
	else if (ratio <= TSL2561_LUX_K3T)
	{b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T;}
	else if (ratio <= TSL2561_LUX_K4T)
	{b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T;}
	else if (ratio <= TSL2561_LUX_K5T)
	{b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T;}
	else if (ratio <= TSL2561_LUX_K6T)
	{b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T;}
	else if (ratio <= TSL2561_LUX_K7T)
	{b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T;}
	else if (ratio > TSL2561_LUX_K8T)
	{b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T;}
#endif

	unsigned long temp;
	temp = ((channel0 * b) - (channel1 * m));

	// do not allow negative lux value
	if (temp < 0)
		temp = 0;

	// round LSB (2^(LUX_SCALE-1))
	temp += (1 << (TSL2561_LUX_LUXSCALE-1));

	// strip off fractional portion
	uint32_t lux = temp >> TSL2561_LUX_LUXSCALE;

	// Signal I2C had no errors
	return lux;
}

uint32_t TSL2561_getFullLuminosity (TSL2561_t *tsl)
{
	if (!tsl->initialized)
		TSL2561_begin(tsl);

	// Enable the device by setting the control bit to 0x03
	TSL2561_enable(tsl);

	// Wait x ms for ADC to complete
	switch (tsl->integration)
	{
		case TSL2561_INTEGRATIONTIME_13MS:
			_delay_ms(14);
			break;
		case TSL2561_INTEGRATIONTIME_101MS:
			_delay_ms(102);
			break;
		default:
			_delay_ms(403);
			break;
	}

	uint32_t x;
	x = TSL2561_read16(tsl, TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
	x <<= 16;
	x |= TSL2561_read16(tsl, TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);

	TSL2561_disable(tsl);

	return x;
}

uint16_t TSL2561_getLuminosity (TSL2561_t *tsl, uint8_t channel)
{
	uint32_t x = TSL2561_getFullLuminosity(tsl);

	if (channel == 0) {
		// Reads two byte value from channel 0 (visible + infrared)
		return (x & 0xFFFF);
	}
	else if (channel == 1) {
		// Reads two byte value from channel 1 (infrared)
		return (x >> 16);
	}
	else if (channel == 2) {
		// Reads all and subtracts out just the visible!
		return ( (x & 0xFFFF) - (x >> 16));
	}

	// unknown channel!
	return 0;
}


uint16_t TSL2561_read16(TSL2561_t *tsl, uint8_t reg)
{
	uint16_t ret;
	uint8_t buffer[2];
	buffer[0] = reg;

	TWI_MasterWriteRead(tsl->twi, tsl->addr, buffer, 1, 2);
	TWI_MasterWait(tsl->twi);

	ret = (tsl->twi->readData[0] << 8) | tsl->twi->readData[1];
	return ret;
}


void TSL2561_write8 (TSL2561_t *tsl, uint8_t reg, uint8_t value)
{
	uint8_t buffer[2];
	buffer[0] = reg;
	buffer[1] = value;

	TWI_MasterWrite(tsl->twi, tsl->addr, buffer, 2);
	TWI_MasterWait(tsl->twi);
}

