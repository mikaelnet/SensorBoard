/*
 * logger.c
 *
 * Created: 2014-03-27 21:56:38
 *  Author: mikael
 */

#include "logger.h"

#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

Process_t logger_process;

bool logger_parse (const char *cmd)
{

    return false;
}


void logger_loop ()
{

}

void logger_init()
{
    process_register(&logger_process, &logger_loop, &logger_parse, NULL);
}