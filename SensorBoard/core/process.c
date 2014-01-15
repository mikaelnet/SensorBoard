/*
 * process.c
 *
 * Created: 2014-01-14 16:27:36
 *  Author: mikael.hogberg
 */ 

#include "process.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static Process_t *processes = NULL;

void process_register (Process_t *process,
    void (*executeLoopMethod)(),
    bool (*parseCommandMethod)(const char *cmd),
    void (*eventHandlerMethod)(EventArgs_t *args))
{
    process->executeLoopMethod = executeLoopMethod;
    process->parseCommandMethod = parseCommandMethod;
    process->eventHandlerMethod = eventHandlerMethod;
    process->next = processes;
    processes = process;
}

void process_raise_event (EventArgs_t *event)
{
    // TODO: Implement an event queue?
    Process_t *ptr = processes;
    while (ptr) {
        if (ptr->eventHandlerMethod != NULL) {
            (ptr->eventHandlerMethod)(event);
        }
        ptr = ptr->next;
    }
}

void process_execute_loop ()
{
    Process_t *ptr = processes;
    while (ptr) {
        if (ptr->executeLoopMethod != NULL) {
            (ptr->executeLoopMethod)();
        }
        ptr = ptr->next;
    }
}

bool process_transmit_command (const char *cmd)
{
    Process_t *ptr = processes;
    while (ptr) {
        if (ptr->parseCommandMethod != NULL) {
            bool result = (ptr->parseCommandMethod)(cmd);
            if (result)
                return true;
        }
        ptr = ptr->next;
    }
    return false;
}

