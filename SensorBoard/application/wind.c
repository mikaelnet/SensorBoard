/*
 * wind.c
 *
 * Created: 2014-01-13 21:38:29
 *  Author: mikael
 */

#include "wind.h"

#include "terminal.h"
#include "../core/process.h"
#include "../drivers/anemometer_driver.h"
#include "../drivers/vane_driver.h"

#include <avr/pgmspace.h>
#include <stdio.h>

static Process_t wind_process;

static uint8_t wind_directions[10];
static uint16_t wind_pulses[10];
static uint8_t wind_data_index;
static uint16_t last_wind_counter;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "WIND";

static void wind_get_wind()
{
    uint16_t avg = wind_pulses[0];
    uint16_t gust = 0;
    for (uint8_t i=1 ; i < 10 ; i++) {
        avg += wind_pulses[i];
        if (wind_pulses[i-1] + wind_pulses[i] > gust) {
            gust = wind_pulses[i-1] + wind_pulses[i];
        }
    }
    float avg_wind = avg/10.0f * 2.4f / 3.6f;
    float gust_wind = gust * 2.4f / 3.6f;

    printf_P(PSTR("Wind speed (gust): %5.2f(%5.2f) m/s\n"), avg_wind, gust_wind);

    // calculate wind direction... this is a bit tricky
    // must loop through all pulses (speed) and directions (indexed)
    // and calculate those as a vector.
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        wind_get_wind();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get wind speed and direction from anemometer and vane"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
}


// call this method every minute (by RTC)
static void wind_minute_pulse()
{
    puts_P(PSTR("Wind"));
    uint16_t c = anemometer_counter();
    wind_pulses[wind_data_index] = c - last_wind_counter;
    //wind_directions[wind_data_index] = vane_calculate;
    wind_data_index ++;
    if (wind_data_index >= 10)
        wind_data_index = 0;
    last_wind_counter = c;

    // TODO: calculate average wind direction

}

static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == MINUTE) {
        wind_minute_pulse();
    }
}

void wind_init()
{
    anemometer_init();
    vane_init();
    wind_data_index = 0;
    for (uint8_t i = 0 ; i < 10 ; i ++)
    {
        wind_pulses[i] = 0;
        wind_directions[i] = -1;
    }
    last_wind_counter = anemometer_counter();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&wind_process, NULL, &event_handler);
}

