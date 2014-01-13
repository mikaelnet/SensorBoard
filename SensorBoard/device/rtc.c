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
// RAIN
ISR(PORTD_INT0_vect)
{
    _minuteInterrupt = true;
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

	// Enable edge interrupt on
	PORTD.INTCTRL = PORT_INT0LVL_MED_gc;
	PORTD.INT0MASK = _BV(2);
	PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
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