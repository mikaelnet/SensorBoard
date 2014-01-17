/*
 * rtc.c
 *
 * Created: 2014-01-10 16:37:49
 *  Author: mikael.hogberg
 */ 

#include "rtc.h"
#include "i2c_bus.h"
#include "../drivers/mcp79410_driver.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

MCP79410_t RTC_Timer;

volatile bool _minuteInterrupt = false;
ISR(PORTD_INT0_vect)
{
    _minuteInterrupt = true;
}


bool RTC_setTime (RTC_DateTime_t *datetime) 
{
    MCP79410_setDate(&RTC_Timer, datetime);
    return true;
}

bool RTC_getTime (RTC_DateTime_t *datetime) 
{
    MCP79410_getDate (&RTC_Timer, datetime);
    return true;
}

bool RTC_is_alarm ()
{
    return _minuteInterrupt;
}

void RTC_reset_alarm ()
{
    MCP79410_resetAlarm0(&RTC_Timer);
    _minuteInterrupt = false;
}

void RTC_start() 
{
    MCP79410_start(&RTC_Timer);
}

void RTC_stop()
{
    MCP79410_stop(&RTC_Timer);
}

void RTC_dump()
{
    MCP79410_dump(&RTC_Timer);
}

// Calculate day of week in proleptic Gregorian calendar. Sunday == 0.
uint8_t RTC_getWeekDay (uint8_t year, uint8_t month, uint8_t day)
{
    int adjustment, mm, yy;
    
    adjustment = (14 - month) / 12;
    mm = month + 12 * adjustment - 2;
    yy = year - adjustment;
    return (day + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

void RTC_init ()
{
    i2c_init();
    MCP79410_init(&RTC_Timer, &i2c);
    
    // Enable alarm0 to trigger every minute
    RTC_DateTime_t alarm;
    alarm.month_bcd = 0;
    alarm.day_bcd = 0;
    alarm.weekday = 0;
    alarm.hours_bcd = 0;
    alarm.minutes_bcd = 0;
    alarm.seconds_bcd = 0;
    MCP79410_setAlarm0 (&RTC_Timer, &alarm, MCP79410_MATCH_SECOND);

    // Enable edge interrupt on PD2
    PORTD.INTCTRL = PORT_INT0LVL_MED_gc;
    PORTD.INT0MASK = _BV(2);
    PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
}


