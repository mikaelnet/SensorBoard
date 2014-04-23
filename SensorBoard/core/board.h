/*
 * board.h
 *
 * Created: 2013-12-30 21:01:24
 *  Author: mikael
 */ 


#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>
#include <stdbool.h>

#define LEDPORT	PORTC
#define GLED_bm	_BV(0)
#define RLED_bm	_BV(1)

#define gled_on()	(LEDPORT.OUTCLR = GLED_bm)
#define gled_off()	(LEDPORT.OUTSET = GLED_bm)
#define rled_on()	(LEDPORT.OUTCLR = RLED_bm)
#define rled_off()	(LEDPORT.OUTSET = RLED_bm)

#ifdef __cplusplus
extern "C" {
#endif

void board_init();

bool button_is_pressed();
void button_reset();

void thsen_enable();
void thsen_disable();
bool thsen_isenabled();
uint16_t thsen_enabledAt();

void ven_enable();
void ven_disable();

void sen_enable();
void sen_disable();

void rfen_enable();
void rfen_disable();

void vrefen_enable();
void vrefen_disable();

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */