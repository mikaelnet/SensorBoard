/*
 * rtc.h
 *
 * Created: 2014-01-10 16:37:33
 *  Author: mikael.hogberg
 */ 


#ifndef RTC_H_
#define RTC_H_

#include "../drivers/mcp79410_driver.h"

typedef struct RTC_DateTime_struct {
    uint8_t year_bcd;
    uint8_t month_bcd;
    uint8_t day_bcd;
    uint8_t dayofweek;
    uint8_t hours_bcd;
    uint8_t minutes_bcd;
    uint8_t seconds_bcd;
} RTC_DateTime_t;

extern bool RTC_setTime (RTC_DateTime_t *datetime);
extern bool RTC_getTime (RTC_DateTime_t *datetime);

#endif /* RTC_H_ */