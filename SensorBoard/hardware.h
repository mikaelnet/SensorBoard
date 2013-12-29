/*
 * hardware.h
 *
 * Created: 2013-12-27 22:18:47
 *  Author: mikael
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

/* 
Compiler options:
=================
Defines:
F_CPU=32000000UL
BAUD=57600


Description of the hardware used: 

Sensors:
========
SEN: PD0 (light/pressure Sensor ENable, active high)
THSEN: PD3 (Temperature/Humidity Sensor ENable, active high)
DS1820: PD5
DHT22: PD4
TSL2561: I2C/TWI on PE0/1 (light sensor on address 0x29 0101001 (gnd), 0x39 0111001 (float) or 0x49 1001001 (high))
BMP085: I2C/TWI on PE0/1 (pressure sensor on address 0xEF/0xEE)

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

RTC:
====
MCP79410
I2C: addr 0x77?
PD2: wake up

MISC:
=====
PA6
PA7


Uträkning av R2 ovan LM4040, 2.5V spänningsreferense
Vref, 2.5V, används som matarspänning till vindflöjeln
Ström genom LM4040 skall vara i området 60uA-15mA
Lägsta möjliga R genom vindflöjeln är vid 112.5 grader, då motståndet är 1k|2k2
Rmin = 5k6 + 1/(1/1000+1/2200) = 5k6 + 687 = 6287 ohm. 
Rmax = 5k6 + 120k = 125k6
Max ström som går genom flöjeln: 2.5V / 6287 ohm ~= 400uA 
Min ström som går genom flöjeln: 2.5V / 125k6 ~= 20uA
Ström genom R2 bör därmed vara minst 500uA, dvs R2 = (3V3-2V5)/500uA = 1k600 ohm minimum
Motsv minsta värde på R2 för att inte överskrida 10mA: (3V3-2V5)/10mA = 80 ohm

Givet 1k, ger I=0.8V/1k=800uA,
-> minst 400uA genom spänningsreferensen (Rmin)
-> max 780uA genom spänningsreferensen (Rmax)

*/

#endif /* HARDWARE_H_ */