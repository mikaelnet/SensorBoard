/*
 * vane_driver.h
 *
 * Created: 2013-12-28 10:56:40
 *  Author: mikael
 */ 


#ifndef VANE_DRIVER_H_
#define VANE_DRIVER_H_

/************************************************************************/
/* Vane                                                                 */
/************************************************************************/

extern void vane_init();
extern int8_t vane_parseReading (uint16_t reading, int16_t *diff); // Make static?

#endif /* VANE_DRIVER_H_ */