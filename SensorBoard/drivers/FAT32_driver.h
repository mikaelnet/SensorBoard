//********************************************************
// **** ROUTINES FOR FAT32 IMPLEMATATION OF SD CARD *****
//********************************************************
//Controller: ATmega32 (Clock: 8 Mhz-internal)
//Compiler	: AVR-GCC (winAVR with AVRStudio)
//Project V.: Version - 2.4.1
//Author	: CC Dharmani, Chennai (India)
//			  www.dharmanitech.com
//Date		: 24 Apr 2011
//********************************************************

//Link to the Post: http://www.dharmanitech.com/2009/01/sd-card-interfacing-with-atmega8-fat32.html

#ifndef _FAT32_DRIVER_H_
#define _FAT32_DRIVER_H_

#include "SD_driver.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

//Structure to access Master Boot Record for getting info about partitions
struct MBRinfo_Structure {
	uint8_t	nothing[446];		//ignore, placed here to fill the gap in the structure
	uint8_t	partitionData[64];	//partition records (16x4)
	uint16_t signature;		//0xaa55
};

//Structure to access info of the first partition of the disk
struct partitionInfo_Structure {
	uint8_t	status;				//0x80 - active partition
	uint8_t headStart;			//starting head
	uint16_t cylSectStart;		//starting cylinder and sector
	uint8_t	type;				//partition type
	uint8_t	headEnd;			//ending head of the partition
	uint16_t cylSectEnd;		//ending cylinder and sector
	uint32_t firstSector;		//total sectors between MBR & the first sector of the partition
	uint32_t sectorsTotal;		//size of this partition in sectors
};

//Structure to access boot sector data
struct BS_Structure {
    uint8_t jumpBoot[3]; //default: 0x009000EB
    uint8_t OEMName[8];
    uint16_t bytesPerSector; //default: 512
    uint8_t sectorPerCluster;
    uint16_t reservedSectorCount;
    uint8_t numberofFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors_F16; //must be 0 for FAT32
    uint8_t mediaType;
    uint16_t FATsize_F16; //must be 0 for FAT32
    uint16_t sectorsPerTrack;
    uint16_t numberofHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors_F32;
    uint32_t FATsize_F32; //count of sectors occupied by one FAT
    uint16_t extFlags;
    uint16_t FSversion; //0x0000 (defines version 0.0)
    uint32_t rootCluster; //first cluster of root directory (=2)
    uint16_t FSinfo; //sector number of FSinfo structure (=1)
    uint16_t BackupBootSector;
    uint8_t reserved[12];
    uint8_t driveNumber;
    uint8_t reserved1;
    uint8_t bootSignature;
    uint32_t volumeID;
    uint8_t volumeLabel[11]; //"NO NAME "
    uint8_t fileSystemType[8]; //"FAT32"
    uint8_t bootData[420];
    uint16_t bootEndSignature; //0xaa55
};


//Structure to access FSinfo sector data
struct FSInfo_Structure {
	uint32_t leadSignature; //0x41615252
	uint8_t reserved1[480];
	uint32_t structureSignature; //0x61417272
	uint32_t freeClusterCount; //initial: 0xffffffff
	uint32_t nextFreeCluster; //initial: 0xffffffff
	uint8_t reserved2[12];
	uint32_t trailSignature; //0xaa550000
};

//Structure to access Directory Entry in the FAT
struct dir_Structure {
	char name[11];
	uint8_t attrib; //file attributes
	uint8_t NTreserved; //always 0
	uint8_t timeTenth; //tenths of seconds, set to 0 here
	uint16_t createTime; //time file was created
	uint16_t createDate; //date file was created
	uint16_t lastAccessDate;
	uint16_t firstClusterHI; //higher word of the first cluster number
	uint16_t writeTime; //time of last write
	uint16_t writeDate; //date of last write
	uint16_t firstClusterLO; //lower word of the first cluster number
	uint32_t fileSize; //size of file in bytes
};

//Attribute definitions for file/directory
#define ATTR_READ_ONLY     0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04
#define ATTR_VOLUME_ID     0x08
#define ATTR_DIRECTORY     0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_LONG_NAME     0x0f


#define DIR_ENTRY_SIZE     0x32
#define EMPTY              0x00
#define DELETED            0xe5
#define GET                0
#define SET                1
#define READ               0
#define VERIFY             1
#define ADD                0
#define REMOVE             1
#define LOW                0
#define HIGH               1
#define TOTAL_FREE         1
#define NEXT_FREE          2
#define GET_LIST           0
#define GET_FILE           1
#define DELETE             2
#define FAT_EOF            0x0fffffff


typedef struct FAT32_FS_struct {
    SD_t *sd;
    FILE *console, *input;
    uint32_t firstDataSector, rootCluster, totalClusters; // volatile
    uint16_t  bytesPerSector, sectorPerCluster, reservedSectorCount; // volatile
    uint32_t unusedSectors, appendFileSector, appendFileLocation, fileSize, appendStartCluster;
    //global flag to keep track of free cluster count updating in FSinfo sector
    bool freeClusterCountUpdated;
} FAT32_FS_t;

//************* functions *************
extern void FAT32_init(FAT32_FS_t *fat32, SD_t *sd, FILE *out, FILE *in);
extern uint8_t FAT32_getBootSectorData (FAT32_FS_t *fat32);
extern uint32_t FAT32_getFirstSector(FAT32_FS_t *fat32, uint32_t clusterNumber);
extern uint32_t FAT32_getSetFreeCluster(FAT32_FS_t *fat32, uint8_t totOrNext, uint8_t get_set, uint32_t FSEntry);
extern struct dir_Structure* FAT32_findFiles (FAT32_FS_t *fat32, uint8_t flag, char *fileName);
extern uint32_t FAT32_getSetNextCluster (FAT32_FS_t *fat32, uint32_t clusterNumber, uint8_t get_set, uint32_t clusterEntry);
extern uint8_t FAT32_readFile (FAT32_FS_t *fat32, uint8_t flag, char *fileName);
extern uint8_t FAT32_convertFileName (char *fileName);
extern void FAT32_writeFile (FAT32_FS_t *fat32, char *fileName);
extern void FAT32_appendFile (FAT32_FS_t *fat32);
extern uint32_t FAT32_searchNextFreeCluster (FAT32_FS_t *fat32, uint32_t startCluster);
extern void FAT32_memoryStatistics (FAT32_FS_t *fat32);
extern void FAT32_displayMemory (FAT32_FS_t *fat32, uint8_t flag, uint32_t memory);
extern void FAT32_deleteFile (FAT32_FS_t *fat32, char *fileName);
extern void FAT32_freeMemoryUpdate (FAT32_FS_t *fat32, uint8_t flag, uint32_t size);

extern bool FAT32_getDateTime (FAT32_FS_t *fat32, uint16_t *date, uint16_t *time);

#endif
