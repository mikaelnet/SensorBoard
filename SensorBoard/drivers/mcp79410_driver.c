/*
 * mcp79410_driver.cpp
 *
 * Created: 2013-12-30 21:35:55
 *  Author: mikael
 */ 

#if MCP79410_ENABLE==1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <alloca.h>
#include <stdbool.h>
#include <string.h>

#include "mcp79410_driver.h"

static uint8_t txBuffer[2];
static uint8_t regsBuffer[0x20];

static uint8_t readReg (TWI_Master_t *twi, uint8_t addr)
{
    txBuffer[0] = addr;
    TWI_MasterWriteRead(twi, MCP79410_RTC_ADDR, txBuffer, 1, 1);
    TWI_MasterWait(twi);
    return twi->readData[0];
}

static void writeReg (TWI_Master_t *twi, uint8_t adr, uint8_t data)
{
    txBuffer[0] = adr;
    txBuffer[1] = data;
    TWI_MasterWrite(twi, MCP79410_RTC_ADDR, txBuffer, 2);
    TWI_MasterWait(twi);
}


static void readRegs (TWI_Master_t *twi, uint8_t addr, uint8_t len)
{
    txBuffer[0] = addr;
    TWI_MasterWriteRead(twi, MCP79410_RTC_ADDR, txBuffer, 1, len);
    TWI_MasterWait(twi);
    
    uint8_t *ptr = regsBuffer+addr;
    for (uint8_t i=0 ; i < len ; i ++)
        *ptr++ = twi->readData[i];
}

static void writeRegs (TWI_Master_t *twi, uint8_t addr, uint8_t len)
{
    uint8_t *tx = alloca(len+1);
    *tx = addr;
    memcpy(tx+1, regsBuffer+addr, len);
    
    TWI_MasterWrite(twi, MCP79410_RTC_ADDR, tx, len+1);
    TWI_MasterWait(twi);
}

void MCP79410_init (MCP79410_t *rtc, TWI_Master_t *twi)
{
    rtc->twi = twi;
}

void MCP79410_setDate (MCP79410_t *rtc, RTC_DateTime_t *dateTime)
{
    readRegs(rtc->twi, MCP79410_TIME_ADDR, 7);
    uint8_t *ptr = regsBuffer+MCP79410_TIME_ADDR;
    *ptr = (*ptr & 0x80) | (dateTime->seconds_bcd & 0x7F);
    ptr ++;
    *ptr++ = dateTime->minutes_bcd & 0x7F;
    *ptr = (*ptr & 0x40) | (dateTime->hours_bcd & 0x3F);
    ptr ++;
    *ptr = (*ptr & 0xF4) | (dateTime->weekday & 0x07);
    ptr ++;
    *ptr++ = dateTime->day_bcd & 0x3F;
    *ptr = (*ptr & 0xE0) | (dateTime->month_bcd & 0x1F);
    ptr ++;
    *ptr = dateTime->year_bcd;
    writeRegs(rtc->twi, MCP79410_TIME_ADDR, 7);
}

void MCP79410_getDate (MCP79410_t *rtc, RTC_DateTime_t *dateTime)
{
    readRegs(rtc->twi, MCP79410_TIME_ADDR, 7);
    uint8_t *ptr = regsBuffer+MCP79410_TIME_ADDR;
    dateTime->seconds_bcd = (*ptr++) & 0x7F;
    dateTime->minutes_bcd = (*ptr++) & 0x7F;
    dateTime->hours_bcd = (*ptr++) & 0x3F;
    dateTime->weekday = (*ptr++) & 0x07;
    dateTime->day_bcd = (*ptr++) & 0x3F;
    dateTime->month_bcd = (*ptr++) & 0x1F;
    dateTime->year_bcd = (*ptr);
}

void MCP79410_start (MCP79410_t *rtc)
{
    uint8_t second = readReg(rtc->twi, MCP79410_START_ADDR);
    second |= MCP79410_START_bm;
    writeReg(rtc->twi, MCP79410_START_ADDR, second);
}

void MCP79410_stop (MCP79410_t *rtc)
{
    uint8_t second = readReg(rtc->twi, MCP79410_START_ADDR);
    second &= ~MCP79410_START_bm;
    writeReg(rtc->twi, MCP79410_START_ADDR, second);
}

static void MCP79410_setAlarm (MCP79410_t *rtc, uint8_t baseAddr, RTC_DateTime_t *dateTime, uint8_t alarmMask)
{
    readRegs(rtc->twi, baseAddr, 6);
    uint8_t *ptr = regsBuffer+baseAddr;
    *ptr = dateTime->seconds_bcd & 0x7F;
    ptr ++;
    *ptr++ = dateTime->minutes_bcd & 0x7F;
    *ptr = (*ptr & 0x40) | (dateTime->hours_bcd & 0x3F);
    ptr ++;
    *ptr++ = (alarmMask & 0xF4) | (dateTime->weekday & 0x07);
    *ptr++ = dateTime->day_bcd & 0x3F;
    *ptr++ = dateTime->month_bcd & 0x1F;
    writeRegs(rtc->twi, baseAddr, 6);
}


void MCP79410_setAlarm0 (MCP79410_t *rtc, RTC_DateTime_t *dateTime, uint8_t alarmMask)
{
    MCP79410_setAlarm (rtc, MCP79410_ALARM0_ADDR, dateTime, alarmMask);
}
void MCP79410_setAlarm1 (MCP79410_t *rtc, RTC_DateTime_t *dateTime, uint8_t alarmMask)
{
    MCP79410_setAlarm (rtc, MCP79410_ALARM1_ADDR, dateTime, alarmMask);
}

static void MCP79410_resetAlarm (TWI_Master_t *twi, uint8_t addr)
{
    uint8_t reg = readReg(twi, addr);
    reg |= 0x01;
    writeReg(twi, addr, reg);
}

void MCP79410_resetAlarm0 (MCP79410_t *rtc)
{
    MCP79410_resetAlarm (rtc->twi, MCP79410_ALARM0_ADDR);
}

void MCP79410_resetAlarm1 (MCP79410_t *rtc)
{
    MCP79410_resetAlarm (rtc->twi, MCP79410_ALARM1_ADDR);
}

//void MCP79410_ReadRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);
//void MCP79410_WriteRAM (MCP79410_t *rtc, char *buf, uint8_t addr, uint8_t len);

#endif