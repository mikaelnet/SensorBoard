/*
 * hardware.h
 *
 * Created: 2013-12-27 22:18:47
 *  Author: mikael
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

/* Description of the hardware used: 

Sensors:
========
SEN: PD0 (light/pressure Sensor ENable, active high)
THSEN: PD3 (Temperature/Humidity Sensor ENable, active high)
DS1820: PD5
DHT22: PD4
TSL2561: I2C/TWI on PE0/1 (light sensor on address x or y)
BMP085: I2C/TWI on PE0/1 (pressure sensor on address)

VREF: PA0, 2.5V
VREFEN: PB0 (active high)
VEN: PB1 (Voltage measure ENable, active high)
V0REF: PA3 (reference GND voltage above N-FET)
VOUT: PA4 (3.3V / 2 above V0REF)
VBAT: PA5 (Battery voltage * 1/3 above V0REF)
VANE: PA1 (0-4095? see separate table)
WIND: PA2 (active falling edge)
RAIN: PB2 (active falling edge)

USB:
====
VBUS: PD1
D-: PD6
D+: PD7

Console:
========
Baud: ??
TX: PE2, RXD0
RX: PE3, TXD0
LED1: PC0, Green (active low)
LED2: PC1, Red (active low)
BTN: PC2, Button (active falling edge)

Antenna:
========
433MHz, length:
Transmit: PC3, TXD0, 4800bps

microSD:
========
SPI, PC5-7
CS: PC4

MISC:
=====
PA6
PA7

*/

#endif /* HARDWARE_H_ */