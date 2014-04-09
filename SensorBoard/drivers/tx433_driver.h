/*
 * tx_driver.h
 *
 * Created: 2014-01-12 22:13:20
 *  Author: mikael
 */


#ifndef TX433_DRIVER_H_
#define TX433_DRIVER_H_

#include "usart_driver.h"



typedef struct TX433_struct {
    USART_t *uart;
} TX433_t;


extern void TX433_init(TX433_t *tx433);
extern void TX433_transmit(TX433_t *tx433, uint8_t *data, uint8_t len);

#endif /* TX433_DRIVER_H_ */