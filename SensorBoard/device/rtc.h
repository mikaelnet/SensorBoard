/*
 * rtc.h
 *
 * Created: 2014-01-10 16:37:33
 *  Author: mikael.hogberg
 */ 


#ifndef RTC_H_
#define RTC_H_

#include <stdbool.h>

#include "time.h"

extern void RTC_init ();
extern bool RTC_setTime (RTC_DateTime_t *datetime);
extern bool RTC_getTime (RTC_DateTime_t *datetime);
extern bool RTC_is_alarm ();
extern void RTC_reset_alarm ();
extern void RTC_start();
extern void RTC_stop();
extern void RTC_dump();
extern uint8_t RTC_getWeekDay (uint8_t year, uint8_t month, uint8_t day);

#endif /* RTC_H_ */