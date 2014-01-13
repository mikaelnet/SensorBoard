/*
 * thermometer.h
 *
 * Created: 2014-01-13 20:32:27
 *  Author: mikael
 */ 


#ifndef THERMOMETER_H_
#define THERMOMETER_H_

#include <stdint.h>
#include <stdbool.h>

extern void thermometer_init();
extern bool thermometer_parse (const char *cmd);
extern bool thermometer_can_sleep();

#endif /* THERMOMETER_H_ */