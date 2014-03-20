/*
 * debug.c
 *
 * Created: 2014-03-20 20:19:43
 *  Author: mikael
 */

#include <stdio.h>
#include <avr/pgmspace.h>

#include "debug.h"

void DEBUG_hexDumpBlock (uint8_t *buf, uint16_t count, FILE *console)
{
    uint8_t *strbuf, *hexbuf;

    for (uint16_t i=0 ; i < count ; i += 16) {
        hexbuf = buf+i;
        for (uint16_t j=i ; j < i+16 ; j ++) {
            if (j < count) {
                fprintf_P(console, PSTR(" %02X"), *hexbuf);
                hexbuf ++;
            }
            else
                fputs_P(PSTR("   "), console);

            if ((j & 0x07) == 7) {
                fputs_P(PSTR("   "), console);
            }
        }
        //fputs_P(PSTR("   "), console);

        strbuf = buf+i;
        for (uint16_t j=i ; j < i+16 && j < count ; j ++) {
            if (*strbuf > 0x20 && *strbuf <= 0x7F)
                fputc(*strbuf, console);
            else
                fputc('.', console);

            if ((j & 0x0F) == 7) {
                fputc(' ', console);
            }
            strbuf ++;
        }
        fputc('\n', console);
    }
}
