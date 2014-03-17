/*
 * ds1820_driver.c
 *
 * Created: 2012-05-20 13:05:28
 *  Author: mikael
 */

#if DS1820_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

#include "ds1820_driver.h"
#include "onewire_driver.h"

#define CONVERT_T   0x44
#define READ        0xBE
#define WRITE       0x4E
#define EE_WRITE    0x48
#define EE_RECALL   0xB8

// Device resolution
#define TEMP_9_BIT  0x1F //  9 bit
#define TEMP_10_BIT 0x3F // 10 bit
#define TEMP_11_BIT 0x5F // 11 bit
#define TEMP_12_BIT 0x7F // 12 bit


bool DS1820_FindFirst(OneWire_t *wire, uint8_t *id)
{
    OneWire_reset_search(wire);
    return DS1820_FindNext(wire, id);
}

bool DS1820_FindNext(OneWire_t *wire, uint8_t *id)
{
    return OneWire_search(wire, id);
}

void DS1820_StartConvertion(OneWire_t *wire, uint8_t *id)
{
    OneWire_reset(wire);
    if (id != NULL)
        OneWire_select(wire, id);
    OneWire_write(wire, CONVERT_T, 0);
}

static int8_t DS1820_GetSensorType (uint8_t *id)
{
    switch(id[0]) {
        case 0x10:
            return 1;	// DS18S20 or old DS1820
            break;
        case 0x28:
            return 0;	// DS18B20
            break;
        case 0x22:
            return 0;	// DS1822
            break;
        default:
            return -1;	// Device is not a DS18x20 family device.
    }
}

// Resolution bits are 9-12.
void DS1820_SetResolution(OneWire_t *wire, uint8_t *id, uint8_t resolutionBits)
{
    // TODO: Complete this method:

    // DS18S20 has fixed 9 bit
    uint8_t configuration;
    if (DS1820_GetSensorType(id) == 0) {
        switch (resolutionBits) {
            case 12:
                configuration = TEMP_12_BIT;
                break;
            case 11:
                configuration = TEMP_11_BIT;
                break;
            case 10:
                configuration = TEMP_10_BIT;
                break;
            case 9:
            default:
                configuration = TEMP_9_BIT;
                break;
        }
        // TODO: Write configuration to scratch pad
        OneWire_reset(wire);
        OneWire_select(wire, id);
        OneWire_write(wire, WRITE, false);         // Read scratch pad
        // TODO: Address is missing here - or we need to write alarms as well
        OneWire_write(wire, configuration, false);
        OneWire_reset(wire);
    }

}

uint16_t DS1820_ReadTemperature(OneWire_t *wire, uint8_t *id)
{
    uint8_t data[12];

    OneWire_reset(wire);
    OneWire_select(wire, id);
    OneWire_write(wire, READ, false);         // Read scratch pad
    for (uint8_t i = 0 ; i < 9 ; i ++) {
        data[i] = OneWire_read(wire);
    }

    int8_t type_s = DS1820_GetSensorType(id);

    uint16_t raw = (data[1] << 8) | data[0];
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
    }

    return true;
}

// return reading / 16.0;
// printf_P(PSTR("%04X  %4d.%01d%cC\n"), temperature, temperature >> 4, (temperature << 12) / 6553, 0xB0);

#endif