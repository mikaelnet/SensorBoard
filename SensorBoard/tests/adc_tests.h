/*
 * adc_tests.h
 *
 * Created: 2013-12-29 22:05:08
 *  Author: mikael
 */ 


#ifndef ADC_TESTS_H_
#define ADC_TESTS_H_

#include <stdbool.h>

extern void irq_tests_setup();
extern bool irq_tests();
extern void adc_tests_setup();
extern bool adc_tests();

#endif /* ADC_TESTS_H_ */