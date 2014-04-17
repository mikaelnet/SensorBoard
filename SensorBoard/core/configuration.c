/*
 * configuration.c
 *
 * Created: 2014-04-17 16:38:41
 *  Author: mikael.hogberg
 */ 

#include "configuration.h"

#include <avr/eeprom.h>

static uint8_t *eeprom_base = (uint8_t *)0;

Configuration_t Configuration;

Configuration_t *config_read()
{
    eeprom_read_block(&Configuration, 0, sizeof(Configuration_t));
    return &Configuration;
}

void config_write()
{
    // Read a shadow copy and write only the changes.
    Configuration_t shadow;
    eeprom_read_block(&shadow, eeprom_base, sizeof(Configuration_t));
    uint8_t *srcPtr = (uint8_t *)(&Configuration);
    uint8_t *dstPtr = (uint8_t *)(&shadow);
    uint8_t *eeptr = eeprom_base;
    for (uint8_t i=0 ; i < sizeof(Configuration_t) ; i ++) {
        if (*srcPtr != *dstPtr) {
            eeprom_write_byte(eeptr, *srcPtr);
        }
        srcPtr ++;
        dstPtr ++;
        eeptr ++;
    }
}



