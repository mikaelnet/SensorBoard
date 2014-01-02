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

#define MCP79410_RTC_ADDR	0x6F
#define MCP79410_EE_ADDR	0x57

typedef struct MCP79410_DateTime_struct
{
	uint8_t reserved_a : 1;
	uint8_t seconds10 : 3;
	uint8_t seconds1 : 4;
	
	uint8_t reserved_b : 1;
	uint8_t minutes10 : 3;
	uint8_t minutes1 : 4;
	
	uint8_t reserved_c : 1;
	uint8_t mode_12_24 : 1;
	uint8_t hour10 : 2;
	uint8_t hour1 : 4;
	
	
} MCP79410_DateTime_t;

typedef struct MCP79410_struct {
	TWI_Master_t *twi;
	
} MCP79410_t;

extern void MCP79410_setDate (MCP79410_t *rtc, MCP79410_DateTime_t *dateTime);
extern void MCP79410_start (MCP79410_t *rtc);
extern void MCP79410_stop (MCP79410_t *rtc);

extern void MCP79410_setAlarm0 (MCP79410_t *rtc, MCP79410_DateTime_t *dateTime, uint8_t alarmMask);
extern void MCP79410_setAlarm1 (MCP79410_t *rtc, MCP79410_DateTime_t *dateTime, uint8_t alarmMask);

extern void MCP79410_ReadRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);
extern void MCP79410_WriteRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);

#endif

#endif /* MCP79410_DRIVER_H_ */