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

extern bool RTC_setTime (RTC_DateTime_t *datetime);
extern bool RTC_getTime (RTC_DateTime_t *datetime);
extern bool RTC_is_alarm ();
extern void RTC_reset_alarm ();

#endif /* RTC_H_ */