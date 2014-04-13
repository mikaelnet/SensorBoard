/*
 * logger.c
 *
 * Created: 2014-03-27 21:56:38
 *  Author: mikael
 */

#include "logger.h"

#include "../core/process.h"
#include "../core/board.h"
#include "terminal.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "LOG";

Process_t logger_process;

static bool parse_command (const char *args)
{
    if (args == NULL || *args == 0 || strcasecmp_P(args, PSTR("GET")) == 0) {
        puts_P(PSTR("Not yet implemented"));
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("Data logger to microSD"));
}

static void print_help ()
{
    puts_P(PSTR("GET   Show current state"));
}


void logger_loop ()
{

}

void logger_init()
{
    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&logger_process, &logger_loop, NULL);
}