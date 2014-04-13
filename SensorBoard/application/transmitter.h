/*
 * transmitter.h
 *
 * Created: 2014-03-27 21:57:37
 *  Author: mikael
 */


#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include <stdint.h>
#include <stdbool.h>

// A summary of data during the last hour
typedef struct TX433_Data_struct {
    // time
    // temperatures (00, 10, 20, 30, 40, 50) - multiple sensors
    // wind avg (00, 10, 20, 30, 40, 50), wind gust, wind direction
    // rain (need time when it occurs)

    uint8_t humidity;
    uint16_t pressure;
    uint16_t light;
    // voltage
    //
    uint8_t crc;
} TX433_Data_t;

void transmitter_debug(const char *data, uint8_t len);
void transmitter_init();

#endif /* TRANSMITTER_H_ */