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
    uint8_t id;
    uint8_t sequence;
    uint8_t contentTypeLength;
    uint8_t body[16];
} TX433_Data_t;

void transmitter_debug(const char *data, uint8_t len);
void transmitter_send(uint8_t sequence, uint8_t contentType, uint8_t contentLength, uint8_t *body);
void transmitter_init();

#endif /* TRANSMITTER_H_ */