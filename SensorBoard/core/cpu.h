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

#ifdef __cplusplus
extern "C" {
#endif

extern void cpu_set_32_MHz();
extern void cpu_set_2_MHz();

extern uint8_t cpu_read_production_signature_byte (uint8_t index);

extern void cpu_init_timer();
extern uint16_t cpu_microsecond();
extern uint16_t cpu_millisecond();
extern uint16_t cpu_second();

extern void cpu_sleep();

#ifdef __cplusplus
}
#endif

#endif /* CPU_H_ */