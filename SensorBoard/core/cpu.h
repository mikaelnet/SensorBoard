/*
 * cpu.h
 *
 * Created: 2013-12-28 00:30:23
 *  Author: mikael
 */ 


#ifndef CPU_H_
#define CPU_H_

#include <inttypes.h>
#include <stdbool.h>

typedef struct CPU_SleepMethod_struct {
    bool (*canSleepMethod)();
    void (*beforeSleepMethod)();
    void (*afterWakeupMethod)();
    struct CPU_SleepMethod_struct *next;
} CPU_SleepMethod_t;

extern void cpu_init();
extern void cpu_set_32_MHz();
extern void cpu_set_2_MHz();

extern uint8_t cpu_read_production_signature_byte (uint8_t index);

extern void cpu_init_timer();
extern uint16_t cpu_microsecond();
extern uint16_t cpu_millisecond();
extern uint16_t cpu_second();

extern void cpu_register_sleep_methods(CPU_SleepMethod_t *holder, 
    bool (*canSleepMethod)(),
    void (*beforeSleepMethod)(), 
    void (*afterWakeupMethod)());
    
extern bool cpu_can_sleep();
extern void cpu_sleep();
extern void cpu_try_sleep();

#endif /* CPU_H_ */