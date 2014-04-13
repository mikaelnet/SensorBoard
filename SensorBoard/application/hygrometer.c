/*
 * hygrometer.c
 *
 * Created: 2014-01-14 16:42:16
 *  Author: mikael.hogberg
 */

#include "hygrometer.h"
#include "../drivers/dht22_driver.h"
#include "terminal.h"

#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdio.h>

static Process_t hygrometer_process;
static DHT22_t dht22;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "HUMIDITY";

static void hygrometer_read()
{
    int16_t temperature, humidity;
    puts_P(PSTR("Reading hygrometer"));
    thsen_enable();
    _delay_ms(2000);

    DHT22_readData(&dht22);
    switch(dht22.error)
    {
        case DHT_ERROR_CHECKSUM:
            puts_P(PSTR("check sum error "));
            //break;

        case DHT_ERROR_NONE:
            temperature = dht22.lastTemperature;
            humidity = dht22.lastHumidity;
            printf_P(PSTR("Temperature: %d.%01d %cC\n"), temperature/10, temperature % 10, 0xB0);
            printf_P(PSTR("Humidity: %d.%01d %% RH\n "), humidity/10, humidity%10);
            int16_t dewPoint = (int16_t)(DHT22_dewPoint(temperature/10, humidity/10)*10);
            printf_P(PSTR("Dewpoint: %d.%01d %cC\n"), dewPoint/10, dewPoint % 10, 0xB0);
            break;

        case DHT_BUS_HUNG:
            puts_P(PSTR("BUS Hung"));
            break;

        case DHT_ERROR_NOT_PRESENT:
            puts_P(PSTR("Not Present"));
            break;

        case DHT_ERROR_ACK_TOO_LONG:
            puts_P(PSTR("ACK time out"));
            break;

        case DHT_ERROR_SYNC_TIMEOUT:
            puts_P(PSTR("Sync Timeout"));
            break;

        case DHT_ERROR_DATA_TIMEOUT:
            puts_P(PSTR("Data Timeout"));
            break;

        case DHT_ERROR_TOOQUICK:
            puts_P(PSTR("Polled to quick"));
            break;
    }

    thsen_disable();
}

static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        hygrometer_read();
        return true;
    }

    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get humidity and temperature from DHT22 hygrometer"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read data"));
}


static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == TENMINUTE) {
        // Here we should ask the RTC what time it is (if unknown)
        // Thereafter, count the number of calls, so we trigger
        // the pulse handler every 60 event
        hygrometer_read();
    }
}


void hygrometer_init ()
{
    DHT22_init(&dht22, &PORTD, 4);

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&hygrometer_process, NULL, &event_handler);
}