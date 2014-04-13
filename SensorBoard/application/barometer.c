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
#include <stdlib.h>
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

    //printf_P(PSTR("ac1-6: %d, %d, %d, %u, %u, %u\n"), bmp085.ac1, bmp085.ac2, bmp085.ac3, bmp085.ac4, bmp085.ac5, bmp085.ac6);
    //printf_P(PSTR("b1, b2: %d, %d\n"), bmp085.b1, bmp085.b2);
    //printf_P(PSTR("mb, mc, md: %d, %d, %d\n"), bmp085.mb, bmp085.mc, bmp085.md);

    int16_t temperature = (int16_t)(BMP085_readTemperature(&bmp085)*10);
    int32_t pressure = BMP085_readPressure(&bmp085);
    int16_t pressure_int = pressure/100;
    int8_t pressure_dec = pressure%100;

    printf_P(PSTR("Temperature: %d.%01d %cC\n"), temperature/10, temperature % 10, 0xB0);
    printf_P(PSTR("Pressure: %d.%02d hPa\n"), pressure_int, pressure_dec);

    if (bmp085.altitude != 0) {
        int32_t psea = (int32_t) BMP085_seaLevelForAltitude (bmp085.altitude, pressure, temperature/10);
        int16_t psea_int = psea / 100;
        int8_t psea_dec = psea % 100;
        printf_P(PSTR("Sea level pressure: %d.%02d hPa"), psea_int, psea_dec);
    }
    //sen_disable();
}


static void set_altitude (int16_t altitude) {
    // set altitude and store in eeprom?
    bmp085.altitude = altitude;
}

static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        barometer_get_pressure();
        return true;
    }
    if (strcasecmp_P(args, PSTR("GET ALT")) == 0) {
        printf_P(PSTR("Altitude is set to %dm over sea level\n"), bmp085.altitude);
        return true;
    }
    if (strncasecmp_P(args, PSTR("SET ALT "), 8) == 0) {
        uint16_t alt = atoi(args+8);
        set_altitude(alt);
        printf_P(PSTR("Altitude set to %dm\n"), bmp085.altitude);
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
    puts_P(PSTR("GET                 Read pressure"));
    puts_P(PSTR("SET ALT [altitude]  Sets altitude in meters over sea level"));
    puts_P(PSTR("GET ALT             Gets specifeid altitude"));
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