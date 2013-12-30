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

#define MCP79410_RTC_ADDR	0x6F
#define MCP79410_EE_ADDR	0x57


#endif

#endif /* MCP79410_DRIVER_H_ */