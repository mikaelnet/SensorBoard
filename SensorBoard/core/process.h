/*
 * process.h
 *
 * Created: 2014-01-14 16:21:46
 *  Author: mikael.hogberg
 */ 


#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum DeviceId_enum {
    DEVICE_CLOCK_ID,
    DEVICE_BUTTON_ID
} DeviceId_t;

typedef enum EventId_enum {
    DEFAULT,
    UNKNOWN
} EventId_t;


typedef struct EventArgs_struct {
    DeviceId_t senderId;
    EventId_t eventId;
    uint16_t eventData;
} EventArgs_t;

typedef struct Process_struct {
    void (*executeLoopMethod)();
    bool (*parseCommandMethod)(const char *cmd);
    void (*eventHandlerMethod)(EventArgs_t *args);
    struct Process_struct *next;
} Process_t;


extern void process_register (Process_t *process,
    void (*executeLoopMethod)(),
    bool (*parseCommandMethod)(const char *cmd),
    void (*eventHandlerMethod)(EventArgs_t *args));

extern void process_raise_event (EventArgs_t *event);
extern void process_execute_loop ();
extern bool process_transmit_command (const char *cmd);

#endif /* PROCESS_H_ */