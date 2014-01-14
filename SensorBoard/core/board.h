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

extern void board_init();
extern bool button_is_pressed();
extern void button_reset();
extern void thsen_enable();
extern void thsen_disable();
extern void ven_enable();
extern void ven_disable();
extern void sen_enable();
extern void sen_disable();
extern void rfen_enable();
extern void rfen_disable();
extern void vrefen_enable();
extern void vrefen_disable();

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */