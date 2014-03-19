/*
 * filesystem.c
 *
 * Created: 2014-03-18 20:09:48
 *  Author: mikael
 */

#include "filesystem.h"

#include <avr/io.h>
#include <avr/pgmspace.h>


#include "../drivers/spi_driver.h"
#include "../drivers/SD_driver.h"
#include "../drivers/FAT32_driver.h"
#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

Process_t filesystem_process;

SPI_Master_t spi;
SD_t sdCard;
FAT32_FS_t fat32_FS;

void filesystem_test()
{
    puts_P(PSTR("SPI init"));
    PORTC.DIRSET = SPI_SS_bm;
    PORTC.PIN4CTRL = PORT_OPC_WIREDANDPULL_gc;
    PORTC.OUTSET = SPI_SS_bm;

    SPI_MasterInit(&spi, &SPIC, &PORTC, false, SPI_MODE_0_gc, false, SPI_PRESCALER_DIV16_gc);
    puts_P(PSTR("SD init"));
    uint8_t result = SD_init(&sdCard, &spi);
    if (result > 0) {
        puts_P(PSTR("Failed to init SD card"));
        printf_P(PSTR("Result %d\n"), result);
        return;
    }

    switch(sdCard.cardType) {
        case 1:
            puts_P(PSTR("Standard Capacity Card (ver 1.x) Detected!"));
            break;
        case 2:
            puts_P(PSTR("High Capacity Card Detected!"));
            break;
        case 3:
            puts_P(PSTR("Standard Capacity Card (ver 2.x) Detected!"));
            break;
        default:
            puts_P(PSTR("Unknown SD Card Detected!"));
            break;
    }

    puts_P(PSTR("FAT32 init"));
    FAT32_init(&fat32_FS, &sdCard, stdout, stdin);

    puts_P(PSTR("SD card boot sector"));
    if (FAT32_getBootSectorData(&fat32_FS) != 0) {
        puts_P(PSTR("Not a FAT32 volume"));
        return;
    }

    printf_P(PSTR("Bytes per sector:    %5d\n"), fat32_FS.bytesPerSector);
    printf_P(PSTR("Sectors per cluster: %5d\n"), fat32_FS.sectorPerCluster);
    printf_P(PSTR("Total clusters:      %lu\n"), fat32_FS.totalClusters);
    printf_P(PSTR("First data sector:   %lu\n"), fat32_FS.firstDataSector);
    printf_P(PSTR("Root cluster:        %lu\n"), fat32_FS.rootCluster);

    // Find files:
    //FAT32_findFiles(&fat32_FS, GET_LIST, "1.txt");
    //FAT32_findFiles(&fat32_FS, GET_LIST, 0);
    //FAT32_readFile(&fat32_FS, READ, "1.txt");

    puts_P(PSTR("Done."));
}

bool filesystem_parse (const char *cmd)
{
    if (strcasecmp_P(cmd, PSTR("FS")) == 0) {
        filesystem_test();
        return true;
    }

    return false;
}

void filesystem_loop ()
{
    // check time if we should calculate temperature. Maybe this should be an event only?
}


void filesystem_init ()
{
    //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
    //clock rate: 125Khz

    process_register(&filesystem_process, &filesystem_loop, &filesystem_parse, NULL);
}

