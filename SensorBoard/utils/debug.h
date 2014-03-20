/*
 * debug.h
 *
 * Created: 2014-03-20 20:19:51
 *  Author: mikael
 */


#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

void DEBUG_hexDumpBlock (uint8_t *buf, uint16_t count, FILE *console);

#endif /* DEBUG_H_ */