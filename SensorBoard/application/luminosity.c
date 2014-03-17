/*
 * luminosity.c
 *
 * Created: 2014-03-17 21:26:37
 *  Author: mikael
 */

#include "luminosity.h"

#include "../device/i2c_bus.h"
#include "../drivers/TSL2561_driver.h"
#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

Process_t luminosity_process;
TSL2561_t tsl;

void luminosity_get_light()
{
    puts_P(PSTR("Reading ambient light"));

    /*if (!TSL2561_begin(&tsl)) {
        puts_P(PSTR("Could not find TSL2561 sensor"));
        return;
    } */

    uint16_t ch0 = TSL2561_getLuminosity(&tsl, TSL2561_FULLSPECTRUM);
    uint16_t ch1 = TSL2561_getLuminosity(&tsl, TSL2561_INFRARED);
    uint16_t ch2 = TSL2561_getLuminosity(&tsl, TSL2561_VISIBLE);
    uint32_t lux = TSL2561_calculateLux(&tsl, ch0, ch1);

    printf_P(PSTR("Full spectrum: %d\n"), ch0);
    printf_P(PSTR("Infrared: %d\n"), ch1);
    printf_P(PSTR("Visible: %d\n"), ch2);
    printf_P(PSTR("lux: %ld\n"), lux);

    puts_P(PSTR("Done."));
}


bool luminosity_parse (const char *cmd)
{
    if (strcasecmp_P(cmd, PSTR("GET LIGHT")) == 0 || strcasecmp_P(cmd, PSTR("LIGHT")) == 0) {
        luminosity_get_light();
        return true;
    }

    return false;
}

void luminosity_loop ()
{
    // check time if we should calculate temperature. Maybe this should be an event only?
}
void luminosity_init()
{
    sen_enable();
    _delay_ms(1000);
    i2c_init();
    TSL2561_init(&tsl, &i2c, TSL2561_ADDR_FLOAT, TSL2561_INTEGRATIONTIME_101MS, TSL2561_GAIN_0X);
    //sen_disable();

    process_register(&luminosity_process, &luminosity_loop, &luminosity_parse, NULL);
}