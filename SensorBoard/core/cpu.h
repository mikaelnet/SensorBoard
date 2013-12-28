/*
 * cpu.h
 *
 * Created: 2013-12-28 00:30:23
 *  Author: mikael
 */ 


#ifndef CPU_H_
#define CPU_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void cpu_set_32_MHz();
extern void cpu_set_2_MHz();

extern unsigned long millis();

#ifdef __cplusplus
}
#endif

#endif /* CPU_H_ */