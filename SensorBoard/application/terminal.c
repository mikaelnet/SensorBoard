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

static Terminal_Command_t *terminalCommands = NULL;

bool terminal_can_sleep()
{
    return cpu_second() - timer > SLEEP_TIMEOUT;
}

static void repeat (char ch, int len)
{
    while (len > 0) {
        putc(ch, stdout);
        len --;
    }
}

void terminal_display_menu() {
    Terminal_Command_t *ptr = terminalCommands;
    int8_t maxLen = 0;
    while (ptr) {
        int8_t len = strlen_P(ptr->name);
        if (len > maxLen)
            maxLen = len;
        ptr = ptr->next;
    }

    puts_P(PSTR("\n\n\n"));
    ptr = terminalCommands;
    while (ptr) {
        repeat(' ', 5);
        fputs_P(ptr->name, stdout);
        repeat(' ', maxLen + 3 - strlen_P(ptr->name));
        if (ptr->menuPrintMethod != NULL) {
            ptr->menuPrintMethod();
        }
        ptr = ptr->next;
    }
    repeat(' ', 5);
    fputs_P(PSTR("     SLEEP"), stdout);
    repeat(' ', maxLen + 3 - 5);
    puts_P(PSTR("Go to sleep"));
    fputs_P(PSTR("     MENU"), stdout);
    repeat(' ', maxLen + 3 - 4);
    puts_P(PSTR("Display this menu"));

    puts_P(PSTR("\nType HELP [command] for details\n\n"));
}

void terminal_display_help(const char *command)
{
    Terminal_Command_t *ptr = terminalCommands;
    while (ptr) {
        if (strcasecmp_P(command, ptr->name) == 0) {
            if (ptr->menuHelpMethod != NULL)
                ptr->menuHelpMethod();
            else
                puts_P(PSTR("No detailed help available for command"));
            return;
        }
        ptr = ptr->next;
    }
    // command not found
    puts_P(PSTR("Command not found"));
}

bool terminal_parse (const char *cmd)
{
    if (*cmd == 0 || strcasecmp_P(cmd, PSTR("MENU")) == 0 || strcasecmp_P(cmd, PSTR("HELP")) == 0) {
        terminal_display_menu();
        return true;
    }
    else if (strncasecmp_P(cmd, PSTR("HELP "), 5) == 0) {
        terminal_display_help (cmd + 5);
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

void terminal_execute_command (const char *cmd)
{
    Terminal_Command_t *ptr = terminalCommands;
    while (ptr) {
        if (ptr->parseCommandMethod != NULL) {
            int8_t len = strlen_P(ptr->name);
            const char *argsPtr = NULL;
            if (strcasecmp_P(cmd, ptr->name) == 0) {
                argsPtr = cmd+len;
            }
            else if (strncasecmp_P(cmd, ptr->name, len) == 0 && cmd[len] == ' '){
                argsPtr = cmd+len+1;
            }
            if (argsPtr != NULL) {
                if (!ptr->parseCommandMethod(cmd+len)) {
                    if (ptr->menuHelpMethod != NULL)
                        ptr->menuHelpMethod();
                    puts_P(PSTR("Unknown arguments"));
                }
                return;
            }
        }
        ptr = ptr->next;
    }
    if (!terminal_parse(cmd))
        puts_P(PSTR("Unknown command"));
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
            terminal_execute_command(terminal_buffer);
            //if (!process_transmit_command(terminal_buffer))
            //    puts_P(PSTR("Unknown command"));

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

void terminal_register_command(Terminal_Command_t *terminal_command, const char *commandName_P,
                               void (*menuPrintMethod)(), void (*menuHelpMethod)(), bool (*parseCommandMethod)(const char *cmd))
{
    terminal_command->name = commandName_P;
    terminal_command->menuPrintMethod = menuPrintMethod;
    terminal_command->menuHelpMethod = menuHelpMethod;
    terminal_command->parseCommandMethod = parseCommandMethod;
    terminal_command->next = terminalCommands;
    terminalCommands = terminal_command;
}

void terminal_init()
{
    timer = cpu_second();
    terminal_buffer_ptr = terminal_buffer;
    terminal_buffer_len = 0;

    cpu_register_sleep_methods(&terminal_sleep_methods, &terminal_can_sleep, NULL, NULL);
    process_register(&terminal_process, &terminal_loop, &terminal_parse, &terminal_event_handler);
}
