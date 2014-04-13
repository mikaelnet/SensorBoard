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
#include "terminal.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "LIGHT";

Process_t luminosity_process;
TSL2561_t tsl;

void luminosity_get_light()
{
    puts_P(PSTR("Reading ambient light"));

    /*if (!TSL2561_begin(&tsl)) {
        puts_P(PSTR("Could not find TSL2561 sensor"));
        return;
    } */

    TSL2561_readChannels(&tsl);

    uint16_t ch0 = TSL2561_getLuminosity(&tsl, TSL2561_FULLSPECTRUM);
    uint16_t ch1 = TSL2561_getLuminosity(&tsl, TSL2561_INFRARED);
    uint16_t ch2 = TSL2561_getLuminosity(&tsl, TSL2561_VISIBLE);
    uint32_t lux = TSL2561_calculateLux(&tsl, ch0, ch1);

    printf_P(PSTR("Full spectrum: %u\n"), ch0);
    printf_P(PSTR("Infrared: %u\n"), ch1);
    printf_P(PSTR("Visible: %u\n"), ch2);
    printf_P(PSTR("lux: %lu\n"), lux);

    puts_P(PSTR("Done."));
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        luminosity_get_light();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get visible and infra red light from TSL2561 light sensor"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
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
    TSL2561_init(&tsl, &i2c, TSL2561_ADDR_FLOAT, TSL2561_INTEGRATIONTIME_13MS, TSL2561_GAIN_0X);
    //sen_disable();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&luminosity_process, &luminosity_loop, NULL);
}