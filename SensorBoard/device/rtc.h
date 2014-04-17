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

void RTC_init ();
bool RTC_setTime (RTC_DateTime_t *datetime);
bool RTC_getTime (RTC_DateTime_t *datetime);
bool RTC_is_alarm ();
void RTC_reset_alarm ();
void RTC_start();
void RTC_stop();
void RTC_battery (bool enable);
bool RTC_getBattery ();
void RTC_powerFailTrack (bool enable);
bool RTC_getPowerFailTrack ();
void RTC_dump();
uint8_t RTC_getWeekDay (uint8_t year, uint8_t month, uint8_t day);

#endif /* RTC_H_ */