/*
 * clock.c
 *
 * Created: 2014-01-12 23:09:40
 *  Author: mikael
 */

#include "clock.h"
#include "terminal.h"
#include "../device/rtc.h"
#include "../core/process.h"

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Process_t clock_process;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "TIME";


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
    puts_P(PSTR("Time set"));
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

static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        clock_get_time();
        return true;
    }
    if (strncasecmp_P(args, PSTR("SET 20"), 6) == 0) {
        clock_set_time(args+6);
        return true;
    }
    if (strcasecmp_P(args, PSTR("START")) == 0) {
        RTC_start();
        return true;
    }
    if (strcasecmp_P(args, PSTR("STOP")) == 0) {
        RTC_stop();
        return true;
    }
    if (strcasecmp_P(args, PSTR("BAT")) == 0) {
        fputs_P(PSTR("Backup battery "), stdout);
        if (RTC_getBattery())
            puts_P(PSTR("enabled."));
        else
            puts_P(PSTR("disabled."));
        return true;
    }
    if (strncasecmp_P(args, PSTR("BAT "), 4) == 0) {
        const char *ptr = args + 4;
        if (strcasecmp_P(ptr, PSTR("ON")) == 0 ||
            strcasecmp_P(ptr, PSTR("ENABLE")) == 0 ||
            strcasecmp_P(ptr, PSTR("1")) == 0) {
            RTC_battery(true);
            puts_P(PSTR("Backup battery enabled"));
            return true;
        }
        else if (strcasecmp_P(ptr, PSTR("OFF")) == 0 ||
            strcasecmp_P(ptr, PSTR("DISABLE")) == 0 ||
            strcasecmp_P(ptr, PSTR("0")) == 0) {
            RTC_battery(false);
            puts_P(PSTR("Backup battery disabled"));
            return true;
        }
    }
    if (strcasecmp_P(args, PSTR("PWR")) == 0) {
        fputs_P(PSTR("Power failure tracking "), stdout);
        if (RTC_getPowerFailTrack())
            puts_P(PSTR("enabled."));
        else
            puts_P(PSTR("disabled."));
        return true;
    }
    if (strncasecmp_P(args, PSTR("PWR "), 4) == 0) {
        const char *ptr = args + 4;
        if (strcasecmp_P(ptr, PSTR("ON")) == 0 ||
        strcasecmp_P(ptr, PSTR("ENABLE")) == 0 ||
        strcasecmp_P(ptr, PSTR("1")) == 0) {
            RTC_powerFailTrack(true);
            puts_P(PSTR("Power failure tracking enabled"));
            return true;
        }
        else if (strcasecmp_P(ptr, PSTR("OFF")) == 0 ||
        strcasecmp_P(ptr, PSTR("DISABLE")) == 0 ||
        strcasecmp_P(ptr, PSTR("0")) == 0) {
            RTC_powerFailTrack(false);
            puts_P(PSTR("Power failure tracking disabled"));
            return true;
        }
    }

    if (strcasecmp_P(args, PSTR("DUMP")) == 0) {
        RTC_dump();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("RTC Date and time functions"));
}

static void print_help ()
{
    puts_P(PSTR("SET [time]    Set time in the format \"yyyy-MM-dd HH:mm:ss\""));
    puts_P(PSTR("GET           Get current time"));
    puts_P(PSTR("START         Start RTC"));
    puts_P(PSTR("STOP          Stop RTC"));
    puts_P(PSTR("BAT [on|off]  Enable/disable backup battery. Query if on/off is left out"));
    puts_P(PSTR("PWR [on|off]  Enable/disable power failure tracking. Query if on/off is left out"));
    puts_P(PSTR("DUMP          Raw dump of RTC memory"));
}

EventArgs_t minuteEventArgs;
EventArgs_t tenMinuteEventArgs;
EventArgs_t hourEventArgs;

void clock_loop ()
{
    if (RTC_is_alarm()) {
        RTC_reset_alarm();

        RTC_DateTime_t datetime;
        RTC_getTime(&datetime);
        printf_P(PSTR("%02X:%02X:%02X\n"), datetime.hours_bcd, datetime.minutes_bcd, datetime.seconds_bcd);

        minuteEventArgs.eventData = (datetime.minutes_bcd >> 4) * 10 + (datetime.minutes_bcd & 0x0F);
        process_raise_event(&minuteEventArgs);

        if ( (datetime.minutes_bcd & 0x0F) == 0 ) {
            tenMinuteEventArgs.eventData = datetime.minutes_bcd >> 4;
            process_raise_event(&tenMinuteEventArgs);
        }

        if ( datetime.minutes_bcd == 0 ) {
            hourEventArgs.eventData = ((datetime.hours_bcd >> 4) & 0x03) * 10 + (datetime.hours_bcd & 0x0F);
            process_raise_event(&hourEventArgs);
        }
    }
}

void clock_init ()
{
    RTC_init();
    RTC_dump();

    minuteEventArgs.senderId = DEVICE_CLOCK_ID;
    minuteEventArgs.eventId = MINUTE;
    minuteEventArgs.eventData = 0;

    tenMinuteEventArgs.senderId = DEVICE_CLOCK_ID;
    tenMinuteEventArgs.eventId = TENMINUTE;
    tenMinuteEventArgs.eventData = 0;

    hourEventArgs.senderId = DEVICE_CLOCK_ID;
    hourEventArgs.eventId = HOUR;
    hourEventArgs.eventData = 0;

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&clock_process, &clock_loop, NULL);
}

