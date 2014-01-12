/*
 * terminal.h
 *
 * Created: 2014-01-12 23:06:16
 *  Author: mikael
 */ 


#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdbool.h>

extern void terminal_init();
extern bool terminal_can_sleep();
extern void terminal_process ();

#endif /* TERMINAL_H_ */