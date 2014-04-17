/*
 * configuration.h
 *
 * Created: 2014-04-17 16:37:39
 *  Author: mikael.hogberg
 */ 


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct Configuration_struct {
    uint8_t device_id;
    
    int16_t altitude;
    // connected sensors
    // measure frequencies
    // alarm levels
} Configuration_t;

extern Configuration_t Configuration;

Configuration_t *config_read();
void config_write();



#endif /* CONFIGURATION_H_ */