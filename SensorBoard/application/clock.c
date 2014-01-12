/*
 * clock.c
 *
 * Created: 2014-01-12 23:09:40
 *  Author: mikael
 */ 

#include "../device/rtc.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static bool str2bcd (const char *str, uint8_t *bcd) 
{
    uint8_t v;
    if (!isdigit(*str))
        return false;
    v = *str - '0';
    v <<= 4;
    str ++;
    if (!isdigit(*str))
        return false;
    v |= *str - '0';
    *bcd = v;
    return true;
}

/*static char *bcd2str (uint8_t bcd, char *str)
{
    *str ++ = '0' + (bcd >> 4);
    *str ++ = '0' + (bcd & 0x0F);
    return str;
}*/

static void unknown_format () {
    puts_P(PSTR("Unknown time format"));
}

void clock_set_time (const char *time)
{
    if (strlen (time) < 17) {
        unknown_format();
        return;
    }
    if (time[2] != '-' || time[5] != '-' || time[8] != ' ' ||
        time[11] != ':' || time[14] != ':') {
        unknown_format();
        return;
    }
    
    RTC_DateTime_t datetime;
    if (!str2bcd(time, &datetime.year_bcd) ||
        !str2bcd(time+3, &datetime.month_bcd) ||
        !str2bcd(time+6, &datetime.day_bcd) ||
        !str2bcd(time+9, &datetime.hours_bcd) ||
        !str2bcd(time+12, &datetime.minutes_bcd) ||
        !str2bcd(time+15, &datetime.seconds_bcd)) {
        unknown_format();
        return;
    }
    
    if (!RTC_setTime(&datetime)) {
        puts_P("Unable to set new time");
        return;
    }
}

void clock_get_time ()
{
    RTC_DateTime_t datetime;
    if (!RTC_getTime(&datetime)) {
        puts_P("Unable to get time");
        return;
    }
    
    printf_P(PSTR("Time 20%02X-%02X-%02X %02X:%02X:%02X\n"),
        datetime.year_bcd, datetime.month_bcd, datetime.day_bcd,
        datetime.hours_bcd, datetime.minutes_bcd, datetime.seconds_bcd);
}

bool clock_parse (const char *cmd)
{
    if (strncasecmp_P(cmd, PSTR("SET TIME 20"), 11) == 0) {
        clock_set_time(cmd+11);
        return true;
    }
    if (strcasecmp_P(cmd, PSTR("GET TIME")) == 0 ||
        strcasecmp_P(cmd, PSTR("TIME")) == 0) {
        clock_get_time();
        return true;
    }
    
    return false;
}