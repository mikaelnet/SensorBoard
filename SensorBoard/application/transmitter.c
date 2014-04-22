/*
 * transmitter.c
 *
 * Created: 2014-03-27 21:57:14
 *  Author: mikael
 */

#include "transmitter.h"

#include "../core/process.h"
#include "../core/board.h"
#include "../core/cpu.h"
#include "../core/configuration.h"
#include "../drivers/tx433_driver.h"
#include "terminal.h"

#include <util/delay.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>

static Process_t process;
static CPU_SleepMethod_t sleep_methods;
static TX433_t tx433;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "TRANSMIT";

void transmitter_send (uint8_t sequence, uint8_t contentType, uint8_t contentLength, uint8_t *body)
{
    TX433_Data_t data;

    TX433_enable(&tx433);

    contentLength &= 0x0F;
    data.id = Configuration.device_id;
    data.sequence = sequence;
    data.contentTypeLength = (contentType << 4) | contentLength;
    memcpy (data.body, body, contentLength);
    uint8_t crc = 0, *ptr = body;
    for (uint8_t i=0 ; i < contentLength ; i ++) {
        crc = _crc_ibutton_update(crc, *ptr ++);
    }
    data.body[contentLength] = crc;

    TX433_transmit(&tx433, (uint8_t *)&data, sizeof(data) - 15 + contentLength);
}

void transmitter_debug(const char *data, uint8_t len)
{
    TX433_enable(&tx433);
    TX433_transmit(&tx433, (uint8_t *) data, len);
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("SEND")) == 0) {
        //transmitter_send(0, 0, 0, 0);
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

static bool can_sleep()
{
    return TX433_isBufferEmpty(&tx433);
}

static void before_sleep()
{
    TX433_disable(&tx433);
}

void transmitter_init()
{
    TX433_init(&tx433);

    cpu_register_sleep_methods(&sleep_methods, &can_sleep, &before_sleep, NULL);
    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&process, NULL, NULL);
}

