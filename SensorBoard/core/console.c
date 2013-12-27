/*
 * console.c
 *
 * Created: 2012-05-20 12:31:20
 *  Author: mikael
 */ 

#include "console.h"

#include <avr/interrupt.h>
#include <stdio.h>

#include "../drivers/usart_driver.h"

static USART_data_t USART_data;

//  Receive complete interrupt service routine.
//  Calls the common receive complete handler with pointer to the correct USART
//  as argument.
ISR(USARTE0_RXC_vect)
{
    USART_RXComplete (&USART_data);
}

//  Data register empty  interrupt service routine.
//  Calls the common data register empty complete handler with pointer to the
//  correct USART as argument.
ISR(USARTE0_DRE_vect)
{                     
    USART_DataRegEmpty (&USART_data);
}

#define USART	USARTE0
#define USART_PORT	PORTE

/* Stream */
static int uart_putchar(char c, FILE *stream);
static int uart_getchar(FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

static int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
		uart_putchar('\r', stream);
	while (!USART_TXBuffer_PutByte(&USART_data, c))
		;	// Loop while TX buffer is full
	
	return 0;
}

static int uart_getchar(FILE *stream)
{
	while (!USART_RXBufferData_Available(&USART_data))
		;
	return USART_RXBuffer_GetByte(&USART_data);
}

bool console_hasdata()
{
	return USART_RXBufferData_Available(&USART_data);
}

bool console_txempty()
{
	return USART_TXBuffer_IsEmpty(&USART_data);
}

void console_init () 
{
	/* Kan detta flyttas till usart_driver? */
    // (TX) as output
    USART_PORT.DIRSET   = PIN3_bm;
    // (RX) as input
    USART_PORT.DIRCLR   = PIN2_bm;
	
	// Use USARTx0 and initialize buffers
    USART_InterruptDriver_Initialize (&USART_data, &USART, USART_DREINTLVL_LO_gc);
    
    // USARTx0, 8 Data bits, No Parity, 1 Stop bit
    USART_Format_Set (USART_data.usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
    
    // Enable RXC interrupt
	USART_RxdInterruptLevel_Set (USART_data.usart, USART_RXCINTLVL_LO_gc);
    
	// Set Baudrate to 9600 bps:
	// Use the default I/O clock frequency that is 2 MHz.
	// Do not use the baudrate scale factor
	//
	// Baudrate select = (1/(16*(((I/O clock frequency)/Baudrate)-1)
	//                 = 12
	// Baud rate calculator: http://prototalk.net/forums/showthread.php?t=188
	
#if F_CPU == 2000000UL
	USART_Baudrate_Set(&USART, 769 , -6);	// 9600 @ 2MHz
#elif F_CPU == 32000000UL 
	USART_Baudrate_Set(&USART, 3317, -4);	// 9600 @ 32MHz
#else
#error Unknown CPU speed
#endif

	// Enable both RX and TX.
	USART_Rx_Enable(USART_data.usart);
	USART_Tx_Enable(USART_data.usart);

	// Enable PMIC interrupt level low
	PMIC.CTRL |= PMIC_LOLVLEX_bm;

	stdout = &mystdout;
	stdin = &mystdin;
}