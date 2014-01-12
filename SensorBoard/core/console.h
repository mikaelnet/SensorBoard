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

#ifdef __cplusplus
extern "C" {
#endif
	
extern void console_init ();
extern bool console_hasdata();
extern bool console_txempty();
extern bool console_txcomplete();

extern void console_enable();
extern void console_disable();

#ifdef __cplusplus
}
#endif

#ifndef CONSOLE_BAUD
	#error CONSOLE_BAUD is not defined
#elif CONSOLE_BAUD == 9600
	#define CONSOLE_BSEL	UART_9600_BSEL
	#define CONSOLE_FACTOR	UART_9600_FACTOR
	#define CONSOLE_CLK2X	UART_9600_CLK2X
#elif CONSOLE_BAUD == 19200
	#define CONSOLE_BSEL	UART_19200_BSEL
	#define CONSOLE_FACTOR	UART_19200_FACTOR
	#define CONSOLE_CLK2X	UART_19200_CLK2X
#elif CONSOLE_BAUD == 38400UL
	#define CONSOLE_BSEL	UART_38400_BSEL
	#define CONSOLE_FACTOR	UART_38400_FACTOR
	#define CONSOLE_CLK2X	UART_38400_CLK2X
#elif CONSOLE_BAUD == 57600
	#define CONSOLE_BSEL	UART_57600_BSEL
	#define CONSOLE_FACTOR	UART_57600_FACTOR
	#define CONSOLE_CLK2X	UART_57600_CLK2X
#elif CONSOLE_BAUD == 115200
	#define CONSOLE_BSEL	UART_115200_BSEL
	#define CONSOLE_FACTOR	UART_115200_FACTOR
	#define CONSOLE_CLK2X	UART_115200_CLK2X
#else
	#error CONSOLE_BAUD has an unknown value
#endif

#endif /* CONSOLE_H_ */