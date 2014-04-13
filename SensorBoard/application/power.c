/*
 * power.c
 *
 * Created: 2014-04-13 21:30:37
 *  Author: mikael
 */

#include "power.h"
#include "terminal.h"
#include "../core/board.h"
#include "../core/process.h"
#include "../device/adc.h"

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>

static Process_t power_process;

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "POWER";

static void power_read_voltage()
{
    puts_P(PSTR("Power"));
    adc_enable();

    ven_enable();
    _delay_us(100);

    uint16_t v0ref = adc_read_v0ref();

    uint16_t vbat = adc_read(ADC_CH_MUXPOS_PIN5_gc);
    printf_P(PSTR("VBAT %d, %dmV\n"), vbat, (uint16_t)((float)(vbat-v0ref)*3.0f*2.5f/4.096f));

    // Measure VOUT
    uint16_t vout = adc_read(ADC_CH_MUXPOS_PIN4_gc);
    printf_P(PSTR("VOUT %d, %dmV\n"), vout, (uint16_t)((float)(vout-v0ref)*2.0f*2.5f/4.096f));

    ven_disable();
    adc_disable();
}


static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        power_read_voltage();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Get power measurement"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Read voltage"));
}

static void event_handler (EventArgs_t *args)
{
    if (args->senderId == DEVICE_CLOCK_ID && args->eventId == HOUR) {
        // Here we should ask the RTC what time it is (if unknown)
        // Thereafter, count the number of calls, so we trigger
        // the pulse handler every 60 event
        power_read_voltage();
    }
}

void power_init()
{
    adc_setup();

    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&power_process, NULL, &event_handler);
}