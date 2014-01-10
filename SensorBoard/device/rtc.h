/*
 * rtc.h
 *
 * Created: 2014-01-10 16:37:33
 *  Author: mikael.hogberg
 */ 


#ifndef RTC_H_
#define RTC_H_

#include "../drivers/mcp79410_driver.h"


extern bool RTC_cmdSetTime (const char *cmd);

#endif /* RTC_H_ */