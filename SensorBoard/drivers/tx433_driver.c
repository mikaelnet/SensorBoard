/*
 * tx433_driver.c
 *
 * Created: 2014-01-12 22:13:49
 *  Author: mikael
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "tx433_driver.h"
#include "../core/board.h"
#include "../core/cpu.h"

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

void TX433_init(TX433_t *tx433)
{
    USART_InterruptDriver_Initialize(&TX433_USART_data, &USARTC0, USART_DREINTLVL_LO_gc);

    // (TX) as output
    PORTC.DIRSET = PIN3_bm;
    USART_Format_Set (TX433_USART_data.usart, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);

    // Change this! should be 4800 instead of 9600!!
    USART_Baudrate_Set(&USARTC0, UART_4800_BSEL , UART_4800_FACTOR, UART_4800_CLK2X);

    // Enable TX.
    USART_Tx_Enable(TX433_USART_data.usart);

    // Enable PMIC interrupt level low
    PMIC.CTRL |= PMIC_LOLVLEN_bm;

    //tx433->uart = &USARTC0;
}

void TX433_transmit(TX433_t *tx433, uint8_t *data, uint8_t len)
{
    // wait for the power to stabilize
    if (!tx433->isEnabled)
        TX433_enable(tx433);

    while (cpu_millisecond() - tx433->enabledAt < 100)
        _delay_ms(1);

    // Repeat 3 times in case of disturbances
    for (uint8_t j=0 ; j < 3 ; j ++) {
        // Send a few 0x00. The stop bits will get the receiver in sync
        TX433_putchar(0x00);
        TX433_putchar(0x00);
        TX433_putchar(0x00);
        TX433_putchar(0x00);

        // Send header
        TX433_putchar(0xAA);

        // Send raw data
        for (uint8_t i=0 ; i < len ; i ++)
            TX433_putchar(*data++);
    }
}

inline bool TX433_isBufferEmpty(TX433_t *tx433)
{
    return USART_TXBuffer_IsEmpty(&TX433_USART_data);
}


void TX433_enable(TX433_t *tx433)
{
    if (!tx433->isEnabled) {
        rfen_enable();
        tx433->enabledAt = cpu_millisecond();
        tx433->isEnabled = true;
    }
}

void TX433_disable(TX433_t *tx433)
{
    if (tx433->isEnabled) {
        // Wait for uart
        while (!TX433_isBufferEmpty(tx433))
            ;
        _delay_ms(50);

        // Disable power
        rfen_disable();
        tx433->isEnabled = false;
    }
}

