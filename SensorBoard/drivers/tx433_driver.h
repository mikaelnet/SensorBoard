/*
 * tx_driver.h
 *
 * Created: 2014-01-12 22:13:20
 *  Author: mikael
 */ 


#ifndef TX433_DRIVER_H_
#define TX433_DRIVER_H_

#include "usart_driver.h"

// A summary of data during the last hour
typedef struct TX433_Data_struct {
    // time
    // temperatures
    // wind avg, wind gust, wind direction
    // rain
    uint8_t humidity;
    uint16_t pressure;
    uint16_t light;
} TX433_Data_t;

typedef struct TX433_struct {
    USART_t *uart;
} TX433_t;


extern void TX433_init();
extern void TX433_transmit(TX433_Data_t *data);

#endif /* TX433_DRIVER_H_ */