/*
 * console.c
 *
 * Created: 2012-05-20 12:31:20
 *  Author: mikael
 */ 

#include "console.h"

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "../drivers/usart_driver.h"

#ifdef __cplusplus
extern "C" {
#endif
	
static USART_data_t USART_data;

// Receive complete interrupt service routine.
// Calls the common receive complete handler with pointer to the correct USART
// as argument.
ISR(USARTE0_RXC_vect)
{
    USART_RXComplete (&USART_data);
}

// Data register empty  interrupt service routine.
// Calls the common data register empty complete handler with pointer to the
// correct USART as argument.
ISR(USARTE0_DRE_vect)
{                     
    USART_DataRegEmpty (&USART_data);
}

// This interrupt vector is used to wake up the MCU from
// sleep mode when it receives data on the UART.
ISR(PORTE_INT0_vect)
{
    // Do nothing here. Just wake it up and let the sleep manager
    // re-enable the uart.
    //---
    // Now when the MCU is awake, disable further interrupts
    // and let the UART take care of if.
    //PORTE.INT0MASK &= _BV(2);
}

#define USART		USARTE0
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

bool console_txcomplete()
{
    if (!console_txempty())
        return false;
    // If the buffer is empty, there can still be a character
    // in the DATA register. If so, the TXC and DRE counter are different
    return (USART.STATUS & USART_TXCIF_bm) != 0;
}

// Call this during init and after any wake up etc to re-enable console
void console_enable() 
{
    // Disable (RX) pin change interrupt
    USART_PORT.INT0MASK &= ~_BV(2); 
	// (TX) as output
	USART_PORT.DIRSET   = PIN3_bm;
	// (RX) as input
	USART_PORT.DIRCLR   = PIN2_bm;
    
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
    USART_Baudrate_Set(&USART, CONSOLE_BSEL , CONSOLE_FACTOR, CONSOLE_CLK2X);

    // Enable both RX and TX.
    USART_Rx_Enable(USART_data.usart);
    USART_Tx_Enable(USART_data.usart);

    // Enable PMIC interrupt level low
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
}

// Disable console before going to sleep and enable wake on pin change.
void console_disable()
{
    // wait for UART buffer to empty
    while (!console_txempty())
        ;
    _delay_ms(5);   // Wait for last character
    
    // Disable RXC interrupt
    USART_RxdInterruptLevel_Set (USART_data.usart, USART_RXCINTLVL_OFF_gc);
    
    // Disable both RX and TX.
    USART_Rx_Disable(USART_data.usart);
    USART_Tx_Disable(USART_data.usart);
    
	// (TX) as input
	USART_PORT.DIRCLR = PIN3_bm;

    // Configure port for interrupt awake
    USART_PORT.INTCTRL = PORT_INT0LVL_LO_gc;
    USART_PORT.INT0MASK |= _BV(2); // Enable (RX) pin change interrupt
    USART_PORT.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
}

void console_init () 
{
	// Use USARTx0 and initialize buffers
	USART_InterruptDriver_Initialize (&USART_data, &USART, USART_DREINTLVL_LO_gc);

    console_enable();
    
	stdout = &mystdout;
	stdin = &mystdin;
}

#ifdef __cplusplus
}
#endif