/*
 * tx433_driver.c
 *
 * Created: 2014-01-12 22:13:49
 *  Author: mikael
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "tx433_driver.h"
#include "../core/board.h"

static USART_data_t TX433_USART_data;

ISR(USARTC0_DRE_vect)
{
    USART_DataRegEmpty (&TX433_USART_data);
}

static void TX433_putchar(uint8_t c)
{
    while (!USART_TXBuffer_PutByte(&TX433_USART_data, c))
        ;	// Loop while TX buffer is full
}

void TX433_init()
{
    USART_Format_Set (TX433_USART_data.usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

    // Change this! should be 4800 instead of 9600!!
    USART_Baudrate_Set(&USARTC0, UART_9600_BSEL , UART_9600_FACTOR, UART_9600_CLK2X);

    // Enable TX.
    USART_Tx_Enable(TX433_USART_data.usart);

    // Enable PMIC interrupt level low
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
}

void TX433_transmit(TX433_Data_t *data) 
{
    // Enable power
    rfen_enable();
    
    // wait for the power to stabilize
    _delay_ms(10);
    
    // Send a few 0xAA
    TX433_putchar(0xAA);
    TX433_putchar(0xAA);
    TX433_putchar(0xAA);
    
    // Send raw data
    uint8_t *cptr = (uint8_t *) data;
    for (uint8_t i=0 ; i < sizeof(TX433_Data_t) ; i ++)
        TX433_putchar(*cptr++);
    
    // Wait for uart
    while (!USART_TXBuffer_IsEmpty(&TX433_USART_data))
        ;
    _delay_ms(5);
    
    // Disable power
    rfen_disable();
}
