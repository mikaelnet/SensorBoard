/*
 * mcp79410_driver.h
 *
 * Created: 2013-12-30 21:36:37
 *  Author: mikael
 */ 


#ifndef MCP79410_TESTS_H_
#define MCP79410_TESTS_H_

#if MCP79410_ENABLE==1

#include <avr/io.h>
#include <stdbool.h>

#include "../drivers/mcp79410_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void mcp79410_setup();
extern void mcp79410_tests();

#ifdef __cplusplus
}
#endif
	
#endif

#endif /* MCP79410_DRIVER_H_ */