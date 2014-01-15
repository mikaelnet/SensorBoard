/*
 * rain.c
 *
 * Created: 2014-01-16 00:05:27
 *  Author: mikael
 */ 

#include "rain.h"

#include "../core/process.h"
#include "../drivers/raingauge_driver.h"

#include <avr/pgmspace.h>
#include <stdio.h>

Process_t rain_process;

void rain_get_rain()
{
    printf_P(PSTR("Rain: %5.2f mm"), raingauge_counter()*0.2794f);
}


bool rain_parse(const char *cmd)
{
    if (strcasecmp_P(cmd, PSTR("GET RAIN")) == 0 ||
    strcasecmp_P(cmd, PSTR("RAIN")) == 0) {
        rain_get_rain();
        return true;
    }
    
    return false;
}

// call this method every hour (by RTC)
void rain_hour_pulse()
{
    // just count level
    puts_P(PSTR("Rain"));
}

void rain_event_handler (EventArgs_t *args)
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
    process_register(&rain_process, NULL, &rain_parse, &rain_event_handler);
}
