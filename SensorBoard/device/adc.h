/*
 * adc.h
 *
 * Created: 2014-01-09 16:54:23
 *  Author: mikael.hogberg
 */ 


#ifndef ADC_H_
#define ADC_H_

extern ADC_t *adc;

extern void adc_setup();
extern void adc_enable();
extern void adc_disable();
extern uint16_t adc_read (uint8_t pin);
extern uint16_t adc_read_v0ref();

#endif /* ADC_H_ */