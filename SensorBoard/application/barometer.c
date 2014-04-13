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

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "PRESSURE";

Process_t barometer_process;
BMP085_t bmp085;

void barometer_get_pressure()
{
    sen_enable();
    _delay_ms(1000);
    puts_P(PSTR("Reading pressure"));

    float temperature = BMP085_readTemperature(&bmp085);
    int32_t pressure = BMP085_readPressure(&bmp085);

    printf_P(PSTR("Temp: %10.3f\n"), temperature);
    printf_P(PSTR("Pressure: %10.3f\n"), (float)pressure/(float)100);

    puts_P(PSTR("Done."));
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

void barometer_loop()
{

}

void barometer_init()
{
    sen_enable();
    _delay_ms(1000);
    i2c_init();
    BMP085_init(&bmp085, Standard, &i2c);
    //sen_disable();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&barometer_process, &barometer_loop, NULL);
}