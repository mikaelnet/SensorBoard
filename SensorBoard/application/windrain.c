/*
 * windrain.c
 *
 * Created: 2014-01-13 21:38:29
 *  Author: mikael
 */ 

#include "windrain.h"

#include "../drivers/rain_driver.h"
#include "../drivers/wind_driver.h"
#include "../drivers/vane_driver.h"

#include <avr/pgmspace.h>
#include <stdio.h>

uint8_t wind_directions[10];
uint16_t wind_pulses[10];
uint8_t wind_data_index;
uint16_t last_wind_counter;

void windrain_init()
{
    rain_init();
    wind_init();
    vane_init();
    wind_data_index = 0;
    for (uint8_t i = 0 ; i < 10 ; i ++)
    {
        wind_pulses[i] = 0;
        wind_directions[i] = -1;
    }
    last_wind_counter = wind_counter();
}

void windrain_get_wind()
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

void windrain_get_rain()
{
    printf_P(PSTR("Rain: %5.2f mm"), rain_counter()*0.2794f);
}

bool windrain_parse(const char *cmd)
{
    if (strcasecmp_P(cmd, PSTR("GET WIND")) == 0 ||
    strcasecmp_P(cmd, PSTR("WIND")) == 0) {
        windrain_get_wind();
        return true;
    }
    if (strcasecmp_P(cmd, PSTR("GET RAIN")) == 0 ||
    strcasecmp_P(cmd, PSTR("RAIN")) == 0) {
        windrain_get_rain();
        return true;
    }
    
    return false;
}

// call this method every minute (by RTC)
void winrain_minute_pulse()
{
    uint16_t c = wind_counter();
    wind_pulses[wind_data_index] = c - last_wind_counter;
    wind_data_index ++;
    if (wind_data_index >= 10)
        wind_data_index = 0;
    last_wind_counter = c;
    
    // calculate average wind direction
    
    
}

bool winrain_can_sleep()
{
    return true;
}

