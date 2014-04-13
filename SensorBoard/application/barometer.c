/*
 * barometer.c
 *
 * Created: 2014-01-14 16:42:51
 *  Author: mikael.hogberg
 */

#include "barometer.h"

#include "../device/i2c_bus.h"
#include "../drivers/bmp085_driver.h"
#include "../core/process.h"
#include "../core/board.h"
#include "terminal.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdbool.h>

static Process_t barometer_process;
static BMP085_t bmp085;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "PRESSURE";

static void barometer_get_pressure()
{
    sen_enable();
    _delay_ms(1000);
    puts_P(PSTR("Reading pressure"));

    int16_t temperature = (int16_t)(BMP085_readTemperature(&bmp085)*10);
    double pressure = (double)BMP085_readPressure(&bmp085);
    int16_t pressure_int = (int16_t)pressure;
    int8_t pressure_dec = ((int32_t)(pressure*100))%100;

    printf_P(PSTR("Temperature: %d.%01d %cC\n"), temperature/10, temperature % 10, 0xB0);
    printf_P(PSTR("Pressure: %d.%02d hPa\n"), pressure_int, pressure_dec);
    //sen_disable();
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        barometer_get_pressure();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get barometer pressure and temperature from BMP085 pressure sensor"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
}

static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == DEFAULT) {
        // Here we should ask the RTC what time it is (if unknown)
        // Thereafter, count the number of calls, so we trigger
        // the pulse handler every 60 event
        barometer_get_pressure();
    }
}

void barometer_init()
{
    sen_enable();
    _delay_ms(1000);
    i2c_init();
    BMP085_init(&bmp085, UltraHighres, &i2c);
    //sen_disable();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&barometer_process, NULL, &event_handler);
}