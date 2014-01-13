/*
 * windrain.h
 *
 * Created: 2014-01-13 21:38:45
 *  Author: mikael
 */ 


#ifndef WINDRAIN_H_
#define WINDRAIN_H_

#include <stdint.h>
#include <stdbool.h>

extern void windrain_init();
extern bool windrain_parse(const char *cmd);
extern void winrain_minute_pulse();
extern bool winrain_can_sleep();

#endif /* WINDRAIN_H_ */