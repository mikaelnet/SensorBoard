/*
 * time.h
 *
 * Created: 2014-01-13 00:53:33
 *  Author: mikael
 */ 


#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

typedef struct RTC_DateTime_struct {
    uint8_t year_bcd;
    uint8_t month_bcd;
    uint8_t day_bcd;
    uint8_t weekday;
    uint8_t hours_bcd;
    uint8_t minutes_bcd;
    uint8_t seconds_bcd;
} RTC_DateTime_t;


#endif /* TIME_H_ */