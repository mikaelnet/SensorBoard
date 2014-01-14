/*
 * event.h
 *
 * Created: 2014-01-14 16:21:46
 *  Author: mikael.hogberg
 */ 


#ifndef EVENT_H_
#define EVENT_H_

typedef enum EventResult_enum {
    OK,
    Error
} EventResult_t;

typedef enum EventType_enum {
    None,
    Command,
    Timer
} EventType_t;


typedef struct EventArgs_struct {
    EventType_t type;
    
} EventArgs_t;

typedef EventResult_t (*EventProc) (EventArgs_t *args);


typedef struct EventRegistry_struct {
    EventProc eventProc;
    struct EventRegistry_struct *next;
} EventRegistry_t;

extern void Event_Register (EventRegistry_t *event);

#endif /* EVENT_H_ */