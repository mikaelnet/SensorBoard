/*
 * terminal.h
 *
 * Created: 2014-01-12 23:06:16
 *  Author: mikael
 */


#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdbool.h>
#include <avr/pgmspace.h>

typedef struct Terminal_Command_struct {
    PGM_P name;
    void (*menuPrintMethod)();
    void (*menuHelpMethod)();
    bool (*parseCommandMethod)(const char *args);
    struct Terminal_Command_struct *next;
} Terminal_Command_t;

void terminal_init();
void terminal_register_command(Terminal_Command_t *terminal_command, const char *commandName_P,
                               void (*menuPrintMethod)(), void (*menuHelpMethod)(), bool (*parseCommandMethod)(const char *args));

#endif /* TERMINAL_H_ */