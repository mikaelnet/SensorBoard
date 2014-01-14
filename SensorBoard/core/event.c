/*
 * event.c
 *
 * Created: 2014-01-14 16:27:36
 *  Author: mikael.hogberg
 */ 

#include "event.h"
#include <stddef.h>

static EventRegistry_t *EventRegistry = NULL;

void Event_Register (EventRegistry_t *event)
{
    event->next = EventRegistry;
    EventRegistry = event;
}

