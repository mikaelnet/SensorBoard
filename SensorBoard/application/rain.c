/*
 * rain.c
 *
 * Created: 2014-01-16 00:05:27
 *  Author: mikael
 */

#include "rain.h"

#include "../core/process.h"
#include "../drivers/raingauge_driver.h"
#include "terminal.h"

#include <avr/pgmspace.h>
#include <stdio.h>

static Process_t rain_process;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "RAIN";

static void rain_get_rain()
{
    printf_P(PSTR("Rain: %5.2f mm"), raingauge_counter()*0.2794f);
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        rain_get_rain();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get rain fall from rain gauge"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
}


// call this method every hour (by RTC)
static void rain_hour_pulse()
{
    // just count level
    puts_P(PSTR("Rain"));
}

static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == DEFAULT) {
        // Here we should ask the RTC what time it is (if unknown)
        // Thereafter, count the number of calls, so we trigger
        // the pulse handler every 60 event
        rain_hour_pulse();
    }
}

void rain_init()
{
    raingauge_init();
    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&rain_process, NULL, &event_handler);
}
