/*
 * terminal.c
 *
 * Created: 2014-01-12 22:37:50
 *  Author: mikael
 */

#include "terminal.h"
#include "../core/cpu.h"
#include "../core/process.h"
#include "../core/console.h"

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define SLEEP_TIMEOUT   2*60    // Allow sleep after two idle minutes

static uint16_t timer;
static char terminal_buffer[80];
static char *terminal_buffer_ptr;
static uint8_t terminal_buffer_len;

Process_t terminal_process;
CPU_SleepMethod_t terminal_sleep_methods;


bool terminal_can_sleep()
{
    return cpu_second() - timer > SLEEP_TIMEOUT;
}

void terminal_display_menu() {
    puts_P(PSTR("\n\n\n"));
    puts_P(PSTR("\tTEMP\tGet temperature"));
    puts_P(PSTR("\tHUMIDITY\tGet humidity and temperature"));
    puts_P(PSTR("\tPRESSURE\tGet pressure and temperature"));
    puts_P(PSTR("\tLIGHT\tGet ambient luminosity"));
    puts_P(PSTR("\tTIME\tGet current time"));
    puts_P(PSTR("\tSET TIME yyyy-MM-dd HH:mm:ss"));
    puts_P(PSTR("\tFS\tView filesystem"));
    puts_P(PSTR("\tSLEEP\tGo to sleep"));
    puts_P(PSTR("\tMENU\tDisplay this menu\n"));
}

bool terminal_parse (const char *cmd)
{
    if (*cmd == 0 || strcasecmp_P(cmd, PSTR("MENU")) == 0) {
        terminal_display_menu();
        return true;
    }
    else if (strcasecmp_P(cmd, PSTR("SLEEP")) == 0 ||
             strcasecmp_P(cmd, PSTR("EXIT")) == 0 ||
             strcasecmp_P(cmd, PSTR("QUIT")) == 0) {
        puts_P(PSTR("Sleeping..."));
        timer = cpu_second() - SLEEP_TIMEOUT + 1;
        return true;
    }
    return false;
}

void terminal_event_handler (EventArgs_t *args)
{
    // Process event stuff, such as displaying the menu when the button is pressed.
}

void terminal_loop ()
{
    while (console_hasdata()) {
        timer = cpu_second();

        char ch = fgetc(stdin);
        if (ch == '\b') {
            if (terminal_buffer_len > 0) {
                terminal_buffer_ptr--;
                *terminal_buffer_ptr = 0;
                terminal_buffer_len--;
            }
        }
        else if (ch == '\r' || ch == '\n') {
            *terminal_buffer_ptr = 0;
            if (!process_transmit_command(terminal_buffer))
                puts_P(PSTR("Unknown command"));

            // clear the buffer afterward
            terminal_buffer_ptr = terminal_buffer;
            terminal_buffer_len = 0;
        }
        else if (terminal_buffer_len < sizeof(terminal_buffer)) {
            *terminal_buffer_ptr ++ = ch;
        }
        else {
            // Buffer overflow. Clear it
            terminal_buffer_ptr = terminal_buffer;
            terminal_buffer_len = 0;
        }
    }
}

void terminal_init()
{
    timer = cpu_second();
    terminal_buffer_ptr = terminal_buffer;
    terminal_buffer_len = 0;
    cpu_register_sleep_methods(&terminal_sleep_methods, &terminal_can_sleep, NULL, NULL);
    process_register(&terminal_process, &terminal_loop, &terminal_parse, &terminal_event_handler);
}
