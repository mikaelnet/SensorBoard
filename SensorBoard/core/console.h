/*
 * console.h
 *
 * Created: 2012-05-20 12:31:02
 *  Author: mikael
 */ 


#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <avr/io.h>
#include <stdbool.h>

bool console_hasdata();
bool console_txempty();
void console_init ();

#endif /* CONSOLE_H_ */