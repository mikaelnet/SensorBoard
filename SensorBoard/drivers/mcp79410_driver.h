/*
 * mcp79410_driver.h
 *
 * Created: 2013-12-30 21:36:37
 *  Author: mikael
 */ 


#ifndef MCP79410_DRIVER_H_
#define MCP79410_DRIVER_H_

#if MCP79410_ENABLE==1

#include <avr/io.h>
#include <stdbool.h>

#include "twi_master_driver.h"
#include "../device/time.h"

#define MCP79410_RTC_ADDR	0x6F
#define MCP79410_EE_ADDR	0x57

#define MCP79410_TIME_ADDR      0x00
#define MCP79410_ALARM0_ADDR    0x0A
#define MCP79410_ALARM0IF_ADDR  0x0D
#define MCP79410_ALARM1_ADDR    0x11
#define MCP79410_ALARM1IF_ADDR  0x14
#define MCP79410_POWEROFF_ADDR  0x18
#define MCP79410_POWERON_ADDR   0x1C

#define MCP79410_START_bm       0x80
#define MCP79410_START_ADDR     0x00

#define MCP79410_MATCH_SECOND   (0x00 << 4)
#define MCP79410_MATCH_MINUTE   (0x01 << 4)
#define MCP79410_MATCH_HOUR     (0x02 << 4)
#define MCP79410_MATCH_DAY      (0x03 << 4)
#define MCP79410_MATCH_DATE     (0x04 << 4)
#define MCP79410_MATCH_ALL      (0x07 << 4)

typedef struct MCP79410_struct {
	TWI_Master_t *twi;
} MCP79410_t;

extern void MCP79410_init (MCP79410_t *rtc, TWI_Master_t *twi);
extern void MCP79410_setDate (MCP79410_t *rtc, RTC_DateTime_t *dateTime);
extern void MCP79410_getDate (MCP79410_t *rtc, RTC_DateTime_t *dateTime);
extern void MCP79410_start (MCP79410_t *rtc);
extern void MCP79410_stop (MCP79410_t *rtc);

extern void MCP79410_setAlarm0 (MCP79410_t *rtc, RTC_DateTime_t *dateTime, uint8_t alarmMask);
extern void MCP79410_setAlarm1 (MCP79410_t *rtc, RTC_DateTime_t *dateTime, uint8_t alarmMask);
extern void MCP79410_resetAlarm0 (MCP79410_t *rtc);
extern void MCP79410_resetAlarm1 (MCP79410_t *rtc);

extern void MCP79410_ReadRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);
extern void MCP79410_WriteRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);

extern void MCP79410_dump (MCP79410_t *rtc);

#endif

#endif /* MCP79410_DRIVER_H_ */