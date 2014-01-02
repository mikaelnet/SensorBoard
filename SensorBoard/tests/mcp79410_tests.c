/*
 * mcp79410_driver.cpp
 *
 * Created: 2013-12-30 21:35:55
 *  Author: mikael
 */ 

#if MCP79410_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdio.h>

#include <util/delay.h>

#include "../device/i2c_bus.h"
#include "../core/console.h"

#include "mcp79410_tests.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint8_t txBuffer[2];
static uint8_t rxBuffer[8];

static void writeReg (uint8_t adr, uint8_t data)
{
	txBuffer[0] = adr;
	txBuffer[1] = data;
	TWI_MasterWrite(&i2c, MCP79410_RTC_ADDR, txBuffer, 2);
	TWI_wait(&i2c);
}

bool isFirst;
void mcp79410_setup()
{
	i2c_init();
	
	for (uint8_t i=0 ; i < TWIM_READ_BUFFER_SIZE ; i ++)
		i2c.readData[i] = 0;
	isFirst = true;
}

void mcp79410_first () 
{
	writeReg (0,0);       //STOP RTC
	writeReg (1,0x21);    //MINUTE=21
	writeReg (2,0x22);    //HOUR=22
	writeReg (3,0x09);    //DAY=1(MONDAY) AND VBAT=1
	writeReg (4,0x30);    //DATE=30
	writeReg (5,0x12);    //MONTH=12
	writeReg (6,0x13);    //YEAR=13
	writeReg (0,0x80);    //START RTC, SECOND=00
}

static void readRegs ()
{
	txBuffer[0] = 0;
	TWI_MasterWriteRead(&i2c, MCP79410_RTC_ADDR, txBuffer, 1, 7);
	TWI_wait(&i2c);
	for (uint8_t i=0 ; i < 7 ; i ++)
		rxBuffer[i] = i2c.readData[i];
}

void mcp79410_tests()
{
	if (isFirst) {
		puts_P(PSTR("Initializing RTC with 2013-12-30 22:21:00"));
		while (!console_txempty())
			;
		mcp79410_first();
		isFirst = false;
		return;
	}
	
	puts_P(PSTR("RTC Time:"));
	readRegs();
	putchar('2');
	putchar('0');
	putchar('0'+(rxBuffer[6] >> 4));
	putchar('0'+(rxBuffer[6] & 0x0F));
	putchar('-');
	putchar('0'+((rxBuffer[5] >> 4) & 0x01));
	putchar('0'+(rxBuffer[5] & 0x0F));
	putchar('-');
	putchar('0'+((rxBuffer[4] >> 4) & 0x03));
	putchar('0'+(rxBuffer[4] & 0x0F));

	putchar(' ');
	putchar('0'+((rxBuffer[2] >> 4) & 0x03));
	putchar('0'+(rxBuffer[2] & 0x0F));
	putchar(':');
	putchar('0'+((rxBuffer[1] >> 4) & 0x07));
	putchar('0'+(rxBuffer[1] & 0x0F));
	putchar(':');
	putchar('0'+((rxBuffer[0] >> 4) & 0x07));
	putchar('0'+(rxBuffer[0] & 0x0F));
	putchar('\n');

}

#ifdef __cplusplus
}
#endif



#endif