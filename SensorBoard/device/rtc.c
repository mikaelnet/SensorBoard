/*
 * rtc.c
 *
 * Created: 2014-01-10 16:37:49
 *  Author: mikael.hogberg
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "rtc.h"
#include "../drivers/mcp79410_driver.h"


static bool str2bcd (const char *str, uint8_t *bcd) {
    uint8_t v;
    if (!isdigit(*str))
    return false;
    v = *str - '0';
    v <<= 4;
    str ++;
    if (!isdigit(*str))
    return false;
    v |= *str - '0';
    *bcd = v;
    return true;
}


// Tests if incoming command is in the format:
// TIME yyyy-MM-dd HH:mm:ss
// If so, it sets that time into the RTC and returns true
// otherwise false.
bool RTC_cmdSetTime (const char *cmd)
{
    if (strncasecmp_P(cmd, PSTR("SET TIME 20"), 11) != 0)
        return false;
    
    if (strlen(cmd) < 11 + 17)
        return false;
    
    const char *dateStr = cmd+11;
    const char *timeStr = dateStr+9;
    
    if (dateStr[2] != '-' || dateStr[5] != '-' ||
        timeStr[2] != ':' || timeStr[5] != ':')
        return false;
    
    uint8_t yearBcd, monthBcd, dayBcd;
    if (!str2bcd(dateStr, &yearBcd) ||
        !str2bcd(dateStr+3, &monthBcd) ||
        !str2bcd(dateStr+6, &dayBcd))
        return false;
        
    uint8_t hourBcd, minuteBcd, secondBcd;
    if (!str2bcd(timeStr, &hourBcd) ||
        !str2bcd(timeStr+3, &minuteBcd) ||
        !str2bcd(timeStr+6, &secondBcd))
        return false;
    
    // Set
    
    return true;
}

