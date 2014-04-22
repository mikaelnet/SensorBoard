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
    //USART_t *uart;
    bool isEnabled;
    uint16_t enabledAt;
} TX433_t;


void TX433_init(TX433_t *tx433);
void TX433_transmit(TX433_t *tx433, uint8_t *data, uint8_t len);
bool TX433_isBufferEmpty(TX433_t *tx433);
void TX433_enable(TX433_t *tx433);
void TX433_disable(TX433_t *tx433);

#endif /* TX433_DRIVER_H_ */