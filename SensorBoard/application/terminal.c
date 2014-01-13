/*
 * terminal.c
 *
 * Created: 2014-01-12 22:37:50
 *  Author: mikael
 */ 

#include "terminal.h"
#include "../core/cpu.h"
#include "../core/console.h"
#include "clock.h"
#include "thermometer.h"

#include <avr/pgmspace.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define SLEEP_TIMEOUT   2*60    // Allow sleep after two idle minutes

static uint16_t timer;
static char terminal_buffer[80];
static char *terminal_buffer_ptr;
static uint8_t terminal_buffer_len;

void terminal_init()
{
    timer = cpu_second();
    terminal_buffer_ptr = terminal_buffer;
    terminal_buffer_len = 0;
}

bool terminal_can_sleep()
{
    return cpu_second() - timer > SLEEP_TIMEOUT; 
}

void terminal_display_menu() {
    puts_P(PSTR("\n\n\n"));
    puts_P(PSTR("\tTEMP\tGet temperature"));
    puts_P(PSTR("\tTIME\tGet current time"));
    puts_P(PSTR("\tSET TIME yyyy-MM-dd HH:mm:ss"));
    puts_P(PSTR("\tTEMP\tGet current temperature"));
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

void terminal_parse_command (const char *cmd)
{
    if (terminal_parse(cmd))
        return;
    if (clock_parse(cmd))
        return;
    if (thermometer_parse(cmd))
        return;
        
    puts_P(PSTR("Unknown command"));
}

void terminal_process ()
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
            terminal_parse_command(terminal_buffer);

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
