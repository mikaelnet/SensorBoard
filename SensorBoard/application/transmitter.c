/*
 * transmitter.c
 *
 * Created: 2014-03-27 21:57:14
 *  Author: mikael
 */

#include "transmitter.h"

#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

Process_t transmitter_process;

bool transmitter_parse (const char *cmd)
{

    return false;
}

void transmitter_loop ()
{

}


void transmitter_init()
{
    process_register(&transmitter_process, &transmitter_loop, &transmitter_parse, NULL);
}