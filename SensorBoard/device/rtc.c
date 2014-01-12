/*
 * rtc.c
 *
 * Created: 2014-01-10 16:37:49
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <stdbool.h>

#include "rtc.h"
#include "../drivers/mcp79410_driver.h"


bool RTC_setTime (RTC_DateTime_t *datetime) 
{
    return true;
}


bool RTC_getTime (RTC_DateTime_t *datetime) 
{
    return true;
}