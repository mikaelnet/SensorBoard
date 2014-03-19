//*******************************************************
// **** ROUTINES FOR FAT32 IMPLEMATATION OF SD CARD ****
//**********************************************************
//Controller: ATmega32 (Clock: 8 Mhz-internal)
//Compiler  : AVR-GCC (winAVR with AVRStudio)
//Project V.: Version - 2.4.1
//Author	: CC Dharmani, Chennai (India)
//            www.dharmanitech.com
//Date      : 24 Apr 2011
//********************************************************

//Link to the Post: http://www.dharmanitech.com/2009/01/sd-card-interfacing-with-atmega8-fat32.html

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "FAT32_driver.h"
#include "SD_driver.h"
#include "mcp79410_driver.h"


void FAT32_init(FAT32_FS_t *fat32, SD_t *sd, FILE *out, FILE *in)
{
    fat32->sd = sd;
    fat32->freeClusterCountUpdated = true;
    fat32->console = out;
    fat32->input = in;
}

//***************************************************************************
//Function: to read data from boot sector of SD card, to determine important
//parameters like bytesPerSector, sectorsPerCluster etc.
//Arguments: none
//return: none
//***************************************************************************
uint8_t FAT32_getBootSectorData (FAT32_FS_t *fat32)
{
    struct BS_Structure *bpb; //mapping the buffer onto the structure
    struct MBRinfo_Structure *mbr;
    struct partitionInfo_Structure *partition;
    uint32_t dataSectors;
    SD_t *sd = fat32->sd;

    fat32->unusedSectors = 0;

    SD_readSingleBlock(sd, 0);
    //FAT32_dumpBlock (sd->buffer, 512, fat32->console);
    uint8_t *buf = sd->buffer;
    for (uint8_t i=0 ; i < 90 ; i ++, buf ++) {
        char ch = *buf;
        printf_P(PSTR(" %02X "), ch);
        putc (ch >= 0x20 && ch <= 0x7E ? ch : '.', stdout);
        if (i % 16 == 15)
            putc ('\n', stdout);
    }
    putc ('\n', stdout);

    bpb = (struct BS_Structure *)(sd->buffer);

    if(bpb->jumpBoot[0]!=0xE9 && bpb->jumpBoot[0]!=0xEB) {  //check if it is boot sector
        mbr = (struct MBRinfo_Structure *)(sd->buffer);       //if it is not boot sector, it must be MBR

        if(mbr->signature != 0xaa55)
            return 1;       //if it is not even MBR then it's not FAT32

        partition = (struct partitionInfo_Structure *)(mbr->partitionData);//first partition
        fat32->unusedSectors = partition->firstSector; //the unused sectors, hidden to the FAT

        SD_readSingleBlock(sd, partition->firstSector); //read the bpb sector
        bpb = (struct BS_Structure *)(sd->buffer);
        if(bpb->jumpBoot[0]!=0xE9 && bpb->jumpBoot[0]!=0xEB)
            return 1;
    }

    fat32->bytesPerSector = bpb->bytesPerSector;
    fat32->sectorPerCluster = bpb->sectorPerCluster;
    fat32->reservedSectorCount = bpb->reservedSectorCount;
    fat32->rootCluster = bpb->rootCluster;// + (sector / sectorPerCluster) +1;
    fat32->firstDataSector = bpb->hiddenSectors + fat32->reservedSectorCount + (bpb->numberofFATs * bpb->FATsize_F32);

    dataSectors = bpb->totalSectors_F32 - bpb->reservedSectorCount - ( bpb->numberofFATs * bpb->FATsize_F32);
    fat32->totalClusters = dataSectors / fat32->sectorPerCluster;

    if((FAT32_getSetFreeCluster (fat32, TOTAL_FREE, GET, 0)) > fat32->totalClusters)  //check if FSinfo free clusters count is valid
        fat32->freeClusterCountUpdated = false;
    else
        fat32->freeClusterCountUpdated = true;
    return 0;
}

//***************************************************************************
//Function: to calculate first sector address of any given cluster
//Arguments: cluster number for which first sector is to be found
//return: first sector address
//***************************************************************************
uint32_t FAT32_getFirstSector(FAT32_FS_t *fat32, uint32_t clusterNumber)
{
    return (((clusterNumber - 2) * fat32->sectorPerCluster) + fat32->firstDataSector);
}

//***************************************************************************
//Function: get cluster entry value from FAT to find out the next cluster in the chain
//or set new cluster entry in FAT
//Arguments: 1. current cluster number, 2. get_set (=GET, if next cluster is to be found or = SET,
//if next cluster is to be set 3. next cluster number, if argument#2 = SET, else 0
//return: next cluster number, if if argument#2 = GET, else 0
//****************************************************************************
uint32_t FAT32_getSetNextCluster (FAT32_FS_t *fat32, uint32_t clusterNumber,
                                  uint8_t get_set, uint32_t clusterEntry)
{
    uint16_t FATEntryOffset;
    uint32_t *FATEntryValue;
    uint32_t FATEntrySector;
    uint8_t retry = 0;
    SD_t *sd = fat32->sd;

    // get sector number of the cluster entry in the FAT
    FATEntrySector = fat32->unusedSectors + fat32->reservedSectorCount + ((clusterNumber * 4) / fat32->bytesPerSector) ;

    // get the offset address in that sector number
    FATEntryOffset = (uint16_t) ((clusterNumber * 4) % fat32->bytesPerSector);

    // read the sector into a buffer
    while(retry <10) {
        if (!SD_readSingleBlock(sd, FATEntrySector))
            break;
        retry++;
    }

    // get the cluster address from the buffer
    FATEntryValue = (uint32_t *) &sd->buffer[FATEntryOffset];

    if(get_set == GET)
        return ((*FATEntryValue) & 0x0fffffff);


    *FATEntryValue = clusterEntry;   //for setting new value in cluster entry in FAT

    SD_writeSingleBlock(sd, FATEntrySector);

    return (0);
}

//********************************************************************************************
//Function: to get or set next free cluster or total free clusters in FSinfo sector of SD card
//Arguments: 1.flag:TOTAL_FREE or NEXT_FREE,
//			 2.flag: GET or SET
//			 3.new FS entry, when argument2 is SET; or 0, when argument2 is GET
//return: next free cluster, if arg1 is NEXT_FREE & arg2 is GET
//        total number of free clusters, if arg1 is TOTAL_FREE & arg2 is GET
//		  0xffffffff, if any error or if arg2 is SET
//********************************************************************************************
uint32_t FAT32_getSetFreeCluster(FAT32_FS_t *fat32, uint8_t totOrNext, uint8_t get_set, uint32_t FSEntry)
{
    SD_t *sd = fat32->sd;
    struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &sd->buffer;   // Really & here?

    SD_readSingleBlock(sd, fat32->unusedSectors + 1);

    if ( (FS->leadSignature != 0x41615252) ||
         (FS->structureSignature != 0x61417272) ||
         (FS->trailSignature !=0xaa550000)) {
        return 0xffffffff;
    }

    if (get_set == GET) {
        if (totOrNext == TOTAL_FREE)
            return FS->freeClusterCount;
        else // when totOrNext = NEXT_FREE
            return FS->nextFreeCluster;
    }
    else {
        if(totOrNext == TOTAL_FREE)
            FS->freeClusterCount = FSEntry;
        else // when totOrNext = NEXT_FREE
            FS->nextFreeCluster = FSEntry;

        SD_writeSingleBlock(sd, fat32->unusedSectors + 1); //update FSinfo
    }
    return 0xffffffff;
}

void FAT32_dumpBlock (uint8_t *buf, uint16_t count, FILE *console)
{
    uint8_t *strbuf, *hexbuf;
    for (uint16_t i=0 ; i < count ; i += 16) {
        hexbuf = buf+i;
        for (uint8_t j=0 ; j < 16 ; j ++) {
            fprintf_P(console, PSTR(" %02X"), *hexbuf);
            if (j == 7) {
                fputc(' ', console);
                fputc(' ', console);
            }
            hexbuf ++;
        }
        fputc(' ', console);
        fputc(' ', console);

        strbuf = buf+i;
        for (uint8_t j=0 ; j < 16 ; j ++) {
            if (*strbuf > 0x20 && *strbuf <= 0x7F)
                fputc(*strbuf, console);
            else
                fputc('.', console);

            if (j == 7) {
                fputc(' ', console);
            }
            strbuf ++;
        }
        fputc('\n', console);
    }
}

//***************************************************************************
//Function: to get DIR/FILE list or a single file address (cluster number) or to delete a specified file
//Arguments: #1 - flag: GET_LIST, GET_FILE or DELETE #2 - pointer to file name (0 if arg#1 is GET_LIST)
//return: first cluster of the file, if flag = GET_FILE
//        print file/dir list of the root directory, if flag = GET_LIST
//		  Delete the file mentioned in arg#2, if flag = DELETE
//****************************************************************************
struct dir_Structure* FAT32_findFiles (FAT32_FS_t *fat32, uint8_t flag, char *fileName)
{
    uint32_t cluster, sector, firstSector, firstCluster, nextCluster;
    struct dir_Structure *dir;
    uint16_t i;
    uint8_t j;
    SD_t *sd = fat32->sd;
    FILE *console = fat32->console;

    cluster = fat32->rootCluster; //root cluster

    while (1) {
        fprintf_P(console, PSTR("Cluster %lu, "), cluster);
        firstSector = FAT32_getFirstSector (fat32, cluster);
        fprintf_P(console, PSTR("First sector %lu\n"), firstSector);

        for(sector = 0; sector < fat32->sectorPerCluster; sector++) {
            SD_readSingleBlock (sd, firstSector + sector);
            FAT32_dumpBlock (sd->buffer, 512, console);

            for (i=0; i < fat32->bytesPerSector; i+=32) {
                dir = (struct dir_Structure *) &sd->buffer[i];

                if (dir->name[0] == EMPTY) {
                    //indicates end of the file list of the directory
                    if ((flag == GET_FILE) || (flag == DELETE))
                        fputs_P(PSTR("File does not exist!"), console);
                    return 0;
                }

                if((dir->name[0] != DELETED) && (dir->attrib != ATTR_LONG_NAME)) {
                    if((flag == GET_FILE) || (flag == DELETE)) {
                        for(j=0; j<11; j++) {
                            if(dir->name[j] != fileName[j])
                                break;
                        }

                        if(j == 11) {
                            if(flag == GET_FILE) {
                                fat32->appendFileSector = firstSector + sector;
                                fat32->appendFileLocation = i;
                                fat32->appendStartCluster = (((uint32_t) dir->firstClusterHI) << 16) | dir->firstClusterLO;
                                fat32->fileSize = dir->fileSize;
                                return dir;
                            }
                            else {   //when flag = DELETE
                                fputs_P(PSTR("Deleting.."), console);
                                firstCluster = (((uint32_t) dir->firstClusterHI) << 16) | dir->firstClusterLO;

                                //mark file as 'deleted' in FAT table
                                dir->name[0] = DELETED;
                                SD_writeSingleBlock (sd, firstSector + sector);

                                FAT32_freeMemoryUpdate (fat32, ADD, dir->fileSize);

                                //update next free cluster entry in FSinfo sector
                                cluster = FAT32_getSetFreeCluster (fat32, NEXT_FREE, GET, 0);
                                if(firstCluster < cluster)
                                    FAT32_getSetFreeCluster (fat32, NEXT_FREE, SET, firstCluster);

                                //mark all the clusters allocated to the file as 'free'
                                while(1) {
                                    nextCluster = FAT32_getSetNextCluster (fat32, firstCluster, GET, 0);
                                    FAT32_getSetNextCluster (fat32, firstCluster, SET, 0);
                                    if(nextCluster > 0x0ffffff6) {
                                        fputs_P(PSTR("File deleted!"), console);
                                        return 0;
                                    }
                                    firstCluster = nextCluster;
                                }
                            }
                        }
                    }
                    else {
                        //when flag = GET_LIST
                        for(j=0; j<11; j++) {
                            if(j == 8)
                                fputc(' ', console);
                            fputc(dir->name[j], console);
                        }
                        fputs_P(PSTR("   "), console);
                        if ((dir->attrib != 0x10) && (dir->attrib != 0x08)) {
                            fputs_P (PSTR("FILE   "), console);
                            FAT32_displayMemory (fat32, LOW, dir->fileSize);
                        }
                        else
                            fputs_P ((dir->attrib == 0x10)? PSTR("DIR") : PSTR("ROOT"), console);
                        fputs_P (PSTR("\n"), console);
                    }
                }
            }
        }

        cluster = FAT32_getSetNextCluster (fat32, cluster, GET, 0);

        if(cluster > 0x0ffffff6)
            return 0;
        if(cluster == 0) {
            fputs_P(PSTR("Error in getting cluster"), console);
            return 0;
        }
    }
    return 0;
}

//***************************************************************************
//Function: if flag=READ then to read file from SD card and send contents to UART
//if flag=VERIFY then functions will verify whether a specified file is already existing
//Arguments: flag (READ or VERIFY) and pointer to the file name
//return: 0, if normal operation or flag is READ
//	      1, if file is already existing and flag = VERIFY
//		  2, if file name is incompatible
//***************************************************************************
uint8_t FAT32_readFile (FAT32_FS_t *fat32, uint8_t flag, char *fileName)
{
    struct dir_Structure *dir;
    uint32_t cluster, byteCounter = 0, fileSize, firstSector;
    uint16_t k;
    uint8_t j, error;
    SD_t *sd = fat32->sd;

    error = FAT32_convertFileName (fileName); //convert fileName into FAT format
    if(error)
        return 2;

    dir = FAT32_findFiles (fat32, GET_FILE, fileName); //get the file location
    if (dir == 0)
        return 0;

    if (flag == VERIFY)
        return (1); //specified file name is already existing

    cluster = (((uint32_t) dir->firstClusterHI) << 16) | dir->firstClusterLO;

    fileSize = dir->fileSize;

    while(1) {
        firstSector = FAT32_getFirstSector (fat32, cluster);

        for(j=0; j < fat32->sectorPerCluster; j++) {
            SD_readSingleBlock(sd, firstSector + j);

            for(k=0; k<512; k++) {
                fputc(sd->buffer[k], fat32->console);
                if ((byteCounter++) >= fileSize )
                    return 0;
            }
        }
        cluster = FAT32_getSetNextCluster (fat32, cluster, GET, 0);
        if (cluster == 0) {
            fputs_P(PSTR("Error in getting cluster"), fat32->console);
            return 0;
        }
    }
    return 0;
}

//***************************************************************************
//Function: to convert normal short file name into FAT format
//Arguments: pointer to the file name
//return: 0, if successful else 1.
//***************************************************************************
uint8_t FAT32_convertFileName (char *fileName)
{
    char fileNameFAT[11];
    uint8_t j, k;

    for (j=0; j<12; j++)
        if(fileName[j] == '.')
            break;

    if (j>8) {
        //fputs_P(PSTR("Invalid fileName.."), fat32->console);
        return 1;
    }

    for (k=0; k<j; k++) //setting file name
        fileNameFAT[k] = fileName[k];

    for (k=j; k<=7; k++) //filling file name trail with blanks
        fileNameFAT[k] = ' ';

    j++;
    for(k=8; k<11; k++) { //setting file extension
        if(fileName[j] != 0)
            fileNameFAT[k] = fileName[j++];
        else { //filling extension trail with blanks
            while(k<11)
                fileNameFAT[k++] = ' ';
        }
    }

    for(j=0; j<11; j++) { //converting small letters to caps
        if((fileNameFAT[j] >= 0x61) && (fileNameFAT[j] <= 0x7a))
            fileNameFAT[j] -= 0x20;
    }

    for(j=0; j<11; j++)
        fileName[j] = fileNameFAT[j];

    return 0;
}

//************************************************************************************
//Function: to create a file in FAT32 format in the root directory if given
//			file name does not exist; if the file already exists then append the data
//Arguments: pointer to the file name
//return: none
//************************************************************************************
void FAT32_writeFile (FAT32_FS_t *fat32, char *fileName)
{
    uint8_t j, data, error, fileCreatedFlag = 0, start = 0, appendFile = 0, sectorEndFlag = 0, sector=0;
    uint16_t i, firstClusterHigh=0, firstClusterLow=0;  //value 0 is assigned just to avoid warning in compilation
    struct dir_Structure *dir;
    uint32_t cluster, nextCluster, prevCluster, firstSector, clusterCount, extraMemory;
    SD_t *sd = fat32->sd;

    j = FAT32_readFile (fat32, VERIFY, fileName);

    if(j == 1) {
        fputs_P(PSTR(" File already exists, appending data.."), fat32->console);
        appendFile = 1;
        cluster = fat32->appendStartCluster;
        clusterCount=0;
        while(1) {
            nextCluster = FAT32_getSetNextCluster (fat32, cluster, GET, 0);
            if (nextCluster == FAT_EOF)
                break;
            cluster = nextCluster;
            clusterCount++;
        }

        //last sector number of the last cluster of the file
        sector = (fat32->fileSize - (clusterCount * fat32->sectorPerCluster * fat32->bytesPerSector)) / fat32->bytesPerSector;
        start = 1;
        // appendFile();
        // return;
    }
    else if (j == 2) {
        return; //invalid file name
    }
    else {
        fputs_P(PSTR(" Creating File.."), fat32->console);

        cluster = FAT32_getSetFreeCluster (fat32, NEXT_FREE, GET, 0);
        if(cluster > fat32->totalClusters)
            cluster = fat32->rootCluster;

        cluster = FAT32_searchNextFreeCluster(fat32, cluster);
        if(cluster == 0) {
            fputs_P(PSTR(" No free cluster!"), fat32->console);
            return;
        }

        FAT32_getSetNextCluster(fat32, cluster, SET, FAT_EOF);   //last cluster of the file, marked EOF

        firstClusterHigh = (uint16_t) ((cluster & 0xffff0000) >> 16 );
        firstClusterLow = (uint16_t) ( cluster & 0x0000ffff);
        fat32->fileSize = 0;
    }

    uint32_t startBlock;
    while(1) {
        if(start) {
            start = 0;
            startBlock = FAT32_getFirstSector (fat32, cluster) + sector;
            SD_readSingleBlock (sd, startBlock);
            i = fat32->fileSize % fat32->bytesPerSector;
            j = sector;
        }
        else {
            startBlock = FAT32_getFirstSector (fat32, cluster);
            i=0;
            j=0;
        }

        fputs_P(PSTR(" Enter text (end with ~):"), fat32->console);

        do {
            if(sectorEndFlag == 1) {
                //special case when the last character in previous sector was '\r'
                fputc ('\n', fat32->console);
                sd->buffer[i++] = '\n'; //appending 'Line Feed (LF)' character
                fat32->fileSize++;
            }

            sectorEndFlag = 0;

            data = fgetc(fat32->input);
            if(data == 0x08) {
                //'Back Space' key pressed
                if(i != 0) {
                    fputc(data, fat32->console);
                    fputc(' ', fat32->console);
                    fputc(data, fat32->console);
                    i--;
                    fat32->fileSize--;
                }
                continue;
            }
            fputc(data, fat32->console);
            sd->buffer[i++] = data;
            fat32->fileSize++;
            if(data == '\r') { //'carriage Return (CR)' character
                if(i == 512)
                    sectorEndFlag = 1;  //flag to indicate that the appended '\n' char should be put in the next sector
                else {
                    fputc ('\n', fat32->console);
                    sd->buffer[i++] = '\n'; //appending 'Line Feed (LF)' character
                    fat32->fileSize++;
                }
            }

            if(i >= 512) {
                //though 'i' will never become greater than 512, it's kept here to avoid
                //infinite loop in case it happens to be greater than 512 due to some data corruption
                i=0;
                error = SD_writeSingleBlock (sd, startBlock);
                j++;
                if (j == fat32->sectorPerCluster) {
                    j = 0;
                    break;
                }
                startBlock++;
            }
        } while (data != '~');

        if(data == '~') {
            fat32->fileSize--; //to remove the last entered '~' character
            i--;
            for(;i<512;i++)  //fill the rest of the buffer with 0x00
                sd->buffer[i]= 0x00;
            error = SD_writeSingleBlock (sd, startBlock);
            break;
        }

        prevCluster = cluster;
        cluster = FAT32_searchNextFreeCluster(fat32, prevCluster); //look for a free cluster starting from the current cluster

        if(cluster == 0) {
            fputs_P(PSTR(" No free cluster!"), fat32->console);
            return;
        }

        FAT32_getSetNextCluster(fat32, prevCluster, SET, cluster);
        FAT32_getSetNextCluster(fat32, cluster, SET, FAT_EOF);   //last cluster of the file, marked EOF
    }

    FAT32_getSetFreeCluster (fat32, NEXT_FREE, SET, cluster); //update FSinfo next free cluster entry

    uint16_t dateFAT, timeFAT;
    error = FAT32_getDateTime(fat32, &dateFAT, &timeFAT);    //get current date & time from the RTC
    if (error) {
        dateFAT = 0;
        timeFAT = 0;
    }

    if (appendFile) { //executes this loop if file is to be appended
        SD_readSingleBlock (sd, fat32->appendFileSector);
        dir = (struct dir_Structure *) &sd->buffer[fat32->appendFileLocation];

        dir->lastAccessDate = 0;   //date of last access ignored
        dir->writeTime = timeFAT;  //setting new time of last write, obtained from RTC
        dir->writeDate = dateFAT;  //setting new date of last write, obtained from RTC
        extraMemory = fat32->fileSize - dir->fileSize;
        dir->fileSize = fat32->fileSize;
        SD_writeSingleBlock (sd, fat32->appendFileSector);
        FAT32_freeMemoryUpdate (fat32, REMOVE, extraMemory); //updating free memory count in FSinfo sector;

        fputs_P(PSTR(" File appended!"), fat32->console);

        return;
    }

    //executes following portion when new file is created
    prevCluster = fat32->rootCluster; //root cluster

    while(1) {
        firstSector = FAT32_getFirstSector (fat32, prevCluster);

        for (sector = 0; sector < fat32->sectorPerCluster; sector++) {
            SD_readSingleBlock (sd, firstSector + sector);

            for(i=0; i < fat32->bytesPerSector; i+=32) {
                dir = (struct dir_Structure *) &sd->buffer[i];

                if(fileCreatedFlag) {
                    // to mark last directory entry with 0x00 (empty) mark
                    //indicating end of the directory file list
                    //dir->name[0] = EMPTY;
                    //SD_writeSingleBlock (firstSector + sector);
                    return;
                }

                if ((dir->name[0] == EMPTY) || (dir->name[0] == DELETED)) {
                    //looking for an empty slot to enter file info
                    for (j=0; j<11; j++)
                        dir->name[j] = fileName[j];
                    dir->attrib = ATTR_ARCHIVE; //setting file attribute as 'archive'
                    dir->NTreserved = 0;        //always set to 0
                    dir->timeTenth = 0;         //always set to 0
                    dir->createTime = timeFAT;  //setting time of file creation, obtained from RTC
                    dir->createDate = dateFAT;  //setting date of file creation, obtained from RTC
                    dir->lastAccessDate = 0;    //date of last access ignored
                    dir->writeTime = timeFAT;   //setting new time of last write, obtained from RTC
                    dir->writeDate = dateFAT;   //setting new date of last write, obtained from RTC
                    dir->firstClusterHI = firstClusterHigh;
                    dir->firstClusterLO = firstClusterLow;
                    dir->fileSize = fat32->fileSize;

                    SD_writeSingleBlock (sd, firstSector + sector);
                    fileCreatedFlag = 1;

                    fputs_P(PSTR(" File Created! "), fat32->console);

                    FAT32_freeMemoryUpdate (fat32, REMOVE, fat32->fileSize); //updating free memory count in FSinfo sector
                }
            }
        }

        cluster = FAT32_getSetNextCluster (fat32, prevCluster, GET, 0);

        if(cluster > 0x0ffffff6) {
            if(cluster == FAT_EOF) {
                //this situation will come when total files in root is multiple of (32*sectorPerCluster)
                cluster = FAT32_searchNextFreeCluster(fat32, prevCluster); //find next cluster for root directory entries
                FAT32_getSetNextCluster(fat32, prevCluster, SET, cluster); //link the new cluster of root to the previous cluster
                FAT32_getSetNextCluster(fat32, cluster, SET, FAT_EOF);  //set the new cluster as end of the root directory
            }
            else {
                fputs_P(PSTR("End of Cluster Chain"), fat32->console);
                return;
            }
        }
        if(cluster == 0) {
            fputs_P(PSTR("Error in getting cluster"), fat32->console);
            return;
        }

        prevCluster = cluster;
    }

    return;
}


//***************************************************************************
//Function: to search for the next free cluster in the root directory
//          starting from a specified cluster
//Arguments: Starting cluster
//return: the next free cluster
//****************************************************************
uint32_t FAT32_searchNextFreeCluster (FAT32_FS_t *fat32, uint32_t startCluster)
{
    uint32_t cluster, *value, sector;
    uint8_t i;
    SD_t *sd = fat32->sd;

    startCluster -=  (startCluster % 128);   //to start with the first file in a FAT sector
    for (cluster =startCluster; cluster < fat32->totalClusters; cluster+=128) {
        sector = fat32->unusedSectors + fat32->reservedSectorCount + ((cluster * 4) / fat32->bytesPerSector);
        SD_readSingleBlock(sd, sector);
        for(i=0; i<128; i++) {
            value = (uint32_t *) &sd->buffer[i*4];
            if(((*value) & 0x0fffffff) == 0)
                return(cluster+i);
        }
    }

    return 0;
}

//***************************************************************************
//Function: to display total memory and free memory of SD card, using UART
//Arguments: none
//return: none
//Note: this routine can take upto 15sec for 1GB card (@1MHz clock)
//it tries to read from SD whether a free cluster count is stored, if it is stored
//then it will return immediately. Otherwise it will count the total number of
//free clusters, which takes time
//****************************************************************************
void FAT32_memoryStatistics (FAT32_FS_t *fat32)
{
    uint32_t freeClusters, totalClusterCount, cluster;
    uint32_t totalMemory, freeMemory;
    uint32_t sector, *value;
    uint16_t i;
    SD_t *sd = fat32->sd;

    totalMemory = fat32->totalClusters * fat32->sectorPerCluster / 1024;
    totalMemory *= fat32->bytesPerSector;

    fputs_P(PSTR("Total Memory: "), fat32->console);

    FAT32_displayMemory (fat32, HIGH, totalMemory);
    freeClusters = FAT32_getSetFreeCluster (fat32, TOTAL_FREE, GET, 0);

    if(freeClusters > fat32->totalClusters) {
        fat32->freeClusterCountUpdated = false;
        freeClusters = 0;
        totalClusterCount = 0;
        cluster = fat32->rootCluster;

        while(1) {
            sector = fat32->unusedSectors + fat32->reservedSectorCount + ((cluster * 4) / fat32->bytesPerSector);
            SD_readSingleBlock(sd, sector);
            for (i=0; i<128; i++) {
                value = (uint32_t *) &sd->buffer[i*4];
                if (((*value)& 0x0fffffff) == 0)
                    freeClusters++;;

                totalClusterCount++;
                if (totalClusterCount == (fat32->totalClusters+2))
                    break;
            }
            if(i < 128)
                break;
            cluster+=128;
        }
    }

    if (!fat32->freeClusterCountUpdated)
        FAT32_getSetFreeCluster (fat32, TOTAL_FREE, SET, freeClusters); //update FSinfo next free cluster entry

    fat32->freeClusterCountUpdated = true;  //set flag
    freeMemory = freeClusters * fat32->sectorPerCluster / 1024;
    freeMemory *= fat32->bytesPerSector ;
    fputs_P(PSTR(" Free Memory: "), fat32->console);
    FAT32_displayMemory (fat32, HIGH, freeMemory);
}

//************************************************************
//Function: To convert the uint32_t value of memory into
//          text string and send to UART
//Arguments: 1. uint8_t flag. If flag is HIGH, memory will be displayed in KBytes, else in Bytes.
//			 2. uint32_t memory value
//return: none
//************************************************************
void FAT32_displayMemory (FAT32_FS_t *fat32, uint8_t flag, uint32_t memory)
{
    uint8_t memoryString[] = "              Bytes"; //19 character long string for memory display
    uint8_t i;

    for(i=12; i>0; i--) { //converting freeMemory into ASCII string
        if(i==5 || i==9) {
            memoryString[i-1] = ',';
            i--;
        }
        memoryString[i-1] = (memory % 10) | 0x30;
        memory /= 10;
        if(memory == 0)
            break;
    }

    if(flag == HIGH)
        memoryString[13] = 'K';

    fputs((const char *)memoryString, fat32->console);
}

//********************************************************************
//Function: to delete a specified file from the root directory
//Arguments: pointer to the file name
//return: none
//********************************************************************
void FAT32_deleteFile (FAT32_FS_t *fat32, char *fileName)
{
    uint8_t error;

    error = FAT32_convertFileName (fileName);
    if(error)
        return;

    FAT32_findFiles (fat32, DELETE, fileName);
}

//********************************************************************
//Function: update the free memory count in the FSinfo sector.
//			Whenever a file is deleted or created, this function will be called
//			to ADD or REMOVE clusters occupied by the file
//Arguments: #1.flag ADD or REMOVE #2.file size in Bytes
//return: none
//********************************************************************
void FAT32_freeMemoryUpdate (FAT32_FS_t *fat32, uint8_t flag, uint32_t size)
{
    uint32_t freeClusters;

    //convert file size into number of clusters occupied
    if ((size % 512) == 0)
        size = size / 512;
    else
        size = (size / 512) + 1;
    if ((size % 8) == 0)
        size = size / 8;
    else
        size = (size / 8) + 1;

    if(fat32->freeClusterCountUpdated) {
        freeClusters = FAT32_getSetFreeCluster (fat32, TOTAL_FREE, GET, 0);
        if(flag == ADD)
            freeClusters = freeClusters + size;
        else  //when flag = REMOVE
            freeClusters = freeClusters - size;
        FAT32_getSetFreeCluster (fat32, TOTAL_FREE, SET, freeClusters);
    }
}

bool FAT32_getDateTime (FAT32_FS_t *fat32, uint16_t *date, uint16_t *time)
{
    // Get time from RTC here
    uint8_t year = 14, month = 1, day = 10, hour = 15, minute = 39, second = 45;

    // uint16_t date
    // [0:4] day of month, 1-31
    // [5:8] month of year, 1=Jan, 1-12
    // [9:15] year since 1980, 0-127 (1980-2107)

    *date = ((year+2000-1980) << 9 ) | (month << 5) | (day);

    // uint16_t time
    // [0:4] 2 second count, 0-29 (0-58)
    // [5:10] minutes, (0-59)
    // [11:15] hours, 0-23
    *time = (hour << 11) | (minute << 5) | (second / 2);

    return true;
}
