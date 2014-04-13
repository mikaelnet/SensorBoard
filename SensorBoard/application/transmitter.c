/*
 * transmitter.c
 *
 * Created: 2014-03-27 21:57:14
 *  Author: mikael
 */

#include "transmitter.h"

#include "../core/process.h"
#include "../core/board.h"
#include "../drivers/tx433_driver.h"
#include "terminal.h"

#include <util/delay.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>
#include <stdio.h>

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "TRANSMIT";

Process_t transmitter_process;
static TX433_t tx433;
TX433_Data_t data;

void transmitter_send ()
{
    uint8_t crc = 0;
    uint8_t *ptr = (uint8_t *)&data;
    for (uint8_t i=0 ; i < sizeof(data)-1 ; i ++) {
        crc = _crc_ibutton_update(crc, *ptr ++);
    }
    data.crc = crc;

    TX433_transmit(&tx433, (uint8_t *)&data, sizeof(data));
}

void transmitter_debug(const char *data, uint8_t len)
{
    TX433_transmit(&tx433, (uint8_t *) data, len);
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("SEND")) == 0) {
        transmitter_send();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Transmit collected data over 433MHz"));
}

static void print_help ()
{
    puts_P(PSTR("SEND   Send buffer"));
}


void transmitter_loop ()
{

}

void transmitter_init()
{
    TX433_init(&tx433);

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&transmitter_process, &transmitter_loop, NULL, NULL);
}