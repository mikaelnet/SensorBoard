/*
 * filesystem.c
 *
 * Created: 2014-03-18 20:09:48
 *  Author: mikael
 */

#include "filesystem.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "terminal.h"
#include "../drivers/spi_driver.h"
#include "../drivers/SD_driver.h"
#include "../drivers/FAT32_driver.h"
#include "../core/process.h"
#include "../core/board.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

static Terminal_Command_t command;
static const char command_name[] PROGMEM = "FS";

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
    puts_P(PSTR("\nFind all files"));
    FAT32_findFiles(&fat32_FS, GET_LIST, 0);

    puts_P(PSTR("\nFind a file"));
    struct dir_Structure *f = FAT32_findFiles(&fat32_FS, GET_FILE, "1       TXT");
    if (f != NULL) {
        printf_P(PSTR("1.TXT: %d bytes\n"), f->fileSize);
        printf_P(PSTR("Created: %04d-%02d-%02d\n"),
            1980 + ((f->createDate >> 9) & 0x7F),
            (f->createDate >> 5) & 0x0F,
            f->createDate & 0x1F);
        printf_P(PSTR("Updated: %04d-%02d-%02d\n"),
            1980 + ((f->writeDate >> 9) & 0x7F),
            (f->writeDate >> 5) & 0x0F,
            f->writeDate & 0x1F);
    }
    else {
        puts_P(PSTR("File not found"));
    }

    puts_P(PSTR("\nRead a file"));
    FAT32_readFile(&fat32_FS, READ, "1.TXT");
    FAT32_readFile(&fat32_FS, READ, "2.TXT");

    puts_P(PSTR("\nWrite a file"));
    FAT32_writeFile(&fat32_FS, "2       TXT");

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

static bool parse_command (const char *args)
{
    if (strcasecmp_P(args, PSTR("TEST")) == 0) {
        filesystem_test();
        return true;
    }
    return false;
}

static void print_menu ()
{
    puts_P(PSTR("File system on microSD card"));
}

static void print_help ()
{
    puts_P(PSTR("TEST   Run file system tests"));
}

static void filesystem_loop ()
{
    // check time if we should calculate temperature. Maybe this should be an event only?
}


void filesystem_init ()
{
    //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
    //clock rate: 125Khz
    terminal_register_command(&command, command_name, &print_menu, &print_help, &parse_command);
    process_register(&filesystem_process, &filesystem_loop, NULL);
}

