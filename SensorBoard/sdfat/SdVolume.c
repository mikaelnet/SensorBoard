/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "SdVolume.h"

#include <stdbool.h>
#include <stdint.h>

#include "SdFat.h"
#include "SdFatConfig.h"

//------------------------------------------------------------------------------
// block caches
// use of static functions save a bit of flash - maybe not worth complexity
//

#define CACHE_STATUS_DIRTY      1
#define CACHE_STATUS_FAT_BLOCK  2
#define CACHE_STATUS_MASK       (CACHE_STATUS_DIRTY | CACHE_STATUS_FAT_BLOCK)
#define CACHE_OPTION_NO_READ    4
// value for option argument in cacheFetch to indicate read from cache
#define CACHE_FOR_READ          0
// value for option argument in cacheFetch to indicate write to cache
#define CACHE_FOR_WRITE         CACHE_STATUS_DIRTY
// reserve cache block with no read
#define CACHE_RESERVE_FOR_WRITE (CACHE_STATUS_DIRTY | CACHE_OPTION_NO_READ)


cache_t *SdVolume_cacheAddress(SdVolume_t *volume) {
    return &volume->m_cacheBuffer;
}

uint32_t SdVolume_cacheBlockNumber(SdVolume_t *volume) {
    return volume->m_cacheBlockNumber;
}

static cache_t* SdVolume_cacheFetch(SdVolume_t *volume, uint32_t blockNumber, uint8_t options);
static cache_t* SdVolume_cacheFetchData(SdVolume_t *volume, uint32_t blockNumber, uint8_t options);
static cache_t* SdVolume_cacheFetchFat(SdVolume_t *volume, uint32_t blockNumber, uint8_t options);
static bool SdVolume_cacheSync(SdVolume_t *volume);
static bool SdVolume_cacheWriteData(SdVolume_t *volume);
static bool SdVolume_cacheWriteFat(SdVolume_t *volume);

//------------------------------------------------------------------------------
uint8_t SdVolume_blockOfCluster(SdVolume_t *volume, uint32_t position) {
    return (position >> 9) & (volume->m_blocksPerCluster - 1);
}
static bool SdVolume_fatGet(SdVolume_t *volume, uint32_t cluster, uint32_t* value);
static bool SdVolume_fatPut(SdVolume_t *volume, uint32_t cluster, uint32_t value);
static bool SdVolume_fatPutEOC(SdVolume_t *volume, uint32_t cluster) {
    return SdVolume_fatPut(volume, cluster, 0x0FFFFFFF);
}
static bool SdVolume_isEOC(SdVolume_t *volume, uint32_t cluster) {
    if (volume->m_fatType == 16)
        return cluster >= FAT16EOC_MIN;
    return cluster >= FAT32EOC_MIN;
}
bool SdVolume_readBlock(SdVolume_t *volume, uint32_t block, uint8_t* dst) {
    return Sd2Card_readBlock(volume->m_sdCard, block, dst);
}
bool SdVolume_writeBlock(SdVolume_t *volume, uint32_t block, const uint8_t* dst) {
    return Sd2Card_writeBlock(volume->m_sdCard, block, dst);
}


cache_t* SdVolume_cacheClear(SdVolume_t *volume) {
    if (!SdVolume_cacheSync(volume))
        return 0;
    volume->m_cacheBlockNumber = 0XFFFFFFFF;
    return &volume->m_cacheBuffer;
}

uint8_t SdVolume_blocksPerCluster(SdVolume_t *volume) {
    return volume->m_blocksPerCluster;
}

uint32_t SdVolume_blocksPerFat(SdVolume_t *volume) {
    return volume->m_blocksPerFat;
}

uint32_t SdVolume_clusterCount(SdVolume_t *volume) {
    return volume->m_clusterCount;
}

uint8_t SdVolume_clusterSizeShift(SdVolume_t *volume) {
    return volume->m_clusterSizeShift;
}

uint32_t SdVolume_dataStartBlock(SdVolume_t *volume) {
    return volume->m_dataStartBlock;
}

uint8_t SdVolume_fatCount(SdVolume_t *volume) {
    return volume->m_fatCount;
}

uint32_t SdVolume_fatStartBlock(SdVolume_t *volume) {
    return volume->m_fatStartBlock;
}

uint8_t SdVolume_fatType(SdVolume_t *volume) {
    return volume->m_fatType;
}

uint32_t SdVolume_rootDirEntryCount(SdVolume_t *volume) {
    return volume->m_rootDirEntryCount;
}

uint32_t SdVolume_rootDirStart(SdVolume_t *volume) {
    return volume->m_rootDirStart;
}

Sd2Card_t* SdVolume_sdCard(SdVolume_t *volume) {
    return volume->m_sdCard;
}

bool SdVolume_dbgFat(SdVolume_t *volume, uint32_t n, uint32_t* v) {
    return SdVolume_fatGet(volume, n, v);
}

//------------------------------------------------------------------------------
// find a contiguous group of clusters
bool SdVolume_allocContiguous(SdVolume_t *volume, uint32_t count, uint32_t* curCluster) {
    // start of group
    uint32_t bgnCluster;
    // end of group
    uint32_t endCluster;
    // last cluster of FAT
    uint32_t fatEnd = volume->m_clusterCount + 1;

    // flag to save place to start next search
    bool setStart;

    // set search start cluster
    if (*curCluster) {
        // try to make file contiguous
        bgnCluster = *curCluster + 1;

        // don't save new start location
        setStart = false;
    }
    else {
        // start at likely place for free cluster
        bgnCluster = volume->m_allocSearchStart;

        // save next search start if no holes.
        setStart = true;
    }
    // end of group
    endCluster = bgnCluster;

    // search the FAT for free clusters
    for (uint32_t n = 0;; n++, endCluster++) {
        // can't find space checked all clusters
        if (n >= volume->m_clusterCount) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        // past end - start from beginning of FAT
        if (endCluster > fatEnd) {
            bgnCluster = endCluster = 2;
        }
        uint32_t f;
        if (!SdVolume_fatGet(volume, endCluster, &f)) {
            DBG_FAIL_MACRO;
            goto fail;
        }

        if (f != 0) {
            // don't update search start if unallocated clusters before endCluster.
            if (bgnCluster != endCluster)
                setStart = false;
            // cluster in use try next cluster as bgnCluster
            bgnCluster = endCluster + 1;
        }
        else if ((endCluster - bgnCluster + 1) == count) {
            // done - found space
            break;
        }
    }
    // remember possible next free cluster
    if (setStart)
        volume->m_allocSearchStart = endCluster + 1;

    // mark end of chain
    if (!SdVolume_fatPutEOC(volume, endCluster)) {
        DBG_FAIL_MACRO;
        goto fail;
    }
    // link clusters
    while (endCluster > bgnCluster) {
        if (!SdVolume_fatPut(volume, endCluster - 1, endCluster)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        endCluster--;
    }
    if (*curCluster != 0) {
        // connect chains
        if (!SdVolume_fatPut(volume, *curCluster, bgnCluster)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
    }
    // return first cluster number to caller
    *curCluster = bgnCluster;
    return true;

    fail:
    return false;
}
//==============================================================================
// cache functions
//------------------------------------------------------------------------------
cache_t* SdVolume_cacheFetch(SdVolume_t *volume, uint32_t blockNumber, uint8_t options) {
    return SdVolume_cacheFetchData(volume, blockNumber, options);
}
//------------------------------------------------------------------------------
cache_t* SdVolume_cacheFetchData(SdVolume_t *volume, uint32_t blockNumber, uint8_t options) {
    if (volume->m_cacheBlockNumber != blockNumber) {
        if (!SdVolume_cacheWriteData(volume)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        if (!(options & CACHE_OPTION_NO_READ)) {
            if (!Sd2Card_readBlock(volume->m_sdCard, blockNumber, volume->m_cacheBuffer.data)) {
                DBG_FAIL_MACRO;
                goto fail;
            }
        }
        volume->m_cacheStatus = 0;
        volume->m_cacheBlockNumber = blockNumber;
    }
    volume->m_cacheStatus |= options & CACHE_STATUS_MASK;
    return &volume->m_cacheBuffer;

    fail:
    return 0;
}
//------------------------------------------------------------------------------
cache_t* SdVolume_cacheFetchFat(SdVolume_t *volume, uint32_t blockNumber, uint8_t options) {
    if (volume->m_cacheFatBlockNumber != blockNumber) {
        if (!SdVolume_cacheWriteFat(volume)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        if (!(options & CACHE_OPTION_NO_READ)) {
            if (!Sd2Card_readBlock(volume->m_sdCard, blockNumber, volume->m_cacheFatBuffer.data)) {
                DBG_FAIL_MACRO;
                goto fail;
            }
        }
        volume->m_cacheFatStatus = 0;
        volume->m_cacheFatBlockNumber = blockNumber;
    }
    volume->m_cacheFatStatus |= options & CACHE_STATUS_MASK;
    return &volume->m_cacheFatBuffer;

    fail:
    return 0;
}
//------------------------------------------------------------------------------
bool SdVolume_cacheSync(SdVolume_t *volume) {
    return SdVolume_cacheWriteData(volume) && SdVolume_cacheWriteFat(volume);
}
//------------------------------------------------------------------------------
bool SdVolume_cacheWriteData(SdVolume_t *volume) {
    if (volume->m_cacheStatus & CACHE_STATUS_DIRTY) {
        if (Sd2Card_writeBlock(volume->m_sdCard, volume->m_cacheBlockNumber, volume->m_cacheBuffer.data)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        volume->m_cacheStatus &= ~CACHE_STATUS_DIRTY;
    }
    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
bool SdVolume_cacheWriteFat(SdVolume_t *volume) {
    if (volume->m_cacheFatStatus & CACHE_STATUS_DIRTY) {
        if (!Sd2Card_writeBlock(volume->m_sdCard, volume->m_cacheFatBlockNumber, volume->m_cacheFatBuffer.data)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        // mirror second FAT
        if (volume->m_fatCount > 1) {
            uint32_t lbn = volume->m_cacheFatBlockNumber + volume->m_blocksPerFat;
            if (!Sd2Card_writeBlock(volume->m_sdCard, lbn, volume->m_cacheFatBuffer.data)) {
                DBG_FAIL_MACRO;
                goto fail;
            }
        }
        volume->m_cacheFatStatus &= ~CACHE_STATUS_DIRTY;
    }
    return true;

    fail:
    return false;
}

//------------------------------------------------------------------------------
void SdVolume_cacheInvalidate(SdVolume_t *volume) {
    volume->m_cacheBlockNumber = 0XFFFFFFFF;
    volume->m_cacheStatus = 0;
}
//==============================================================================
//------------------------------------------------------------------------------
uint32_t SdVolume_clusterStartBlock(SdVolume_t *volume, uint32_t cluster) {
  return volume->m_dataStartBlock + ((cluster - 2)*volume->m_blocksPerCluster);
}
//------------------------------------------------------------------------------
// Fetch a FAT entry
bool SdVolume_fatGet(SdVolume_t *volume, uint32_t cluster, uint32_t* value) {
    uint32_t lba;
    cache_t* pc;
    // error if reserved cluster of beyond FAT
    if (cluster < 2  || cluster > (volume->m_clusterCount + 1)) {
        DBG_FAIL_MACRO;
        goto fail;
    }

    if (volume->m_fatType == 16) {
        lba = volume->m_fatStartBlock + (cluster >> 8);
    }
    else if (volume->m_fatType == 32) {
        lba = volume->m_fatStartBlock + (cluster >> 7);
    }
    else {
        DBG_FAIL_MACRO;
        goto fail;
    }

    pc = SdVolume_cacheFetchFat(volume, lba, CACHE_FOR_READ);
    if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
    }
    if (volume->m_fatType == 16) {
        *value = pc->fat16[cluster & 0XFF];
    }
    else {
        *value = pc->fat32[cluster & 0X7F] & FAT32MASK;
    }
    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
// Store a FAT entry
bool SdVolume_fatPut(SdVolume_t *volume, uint32_t cluster, uint32_t value) {
    uint32_t lba;
    cache_t* pc;
    // error if reserved cluster of beyond FAT
    if (cluster < 2 || cluster > (volume->m_clusterCount + 1)) {
        DBG_FAIL_MACRO;
        goto fail;
    }

    if (volume->m_fatType == 16) {
        lba = volume->m_fatStartBlock + (cluster >> 8);
    }
    else if (volume->m_fatType == 32) {
        lba = volume->m_fatStartBlock + (cluster >> 7);
    }
    else {
        DBG_FAIL_MACRO;
        goto fail;
    }

    pc = SdVolume_cacheFetchFat(volume, lba, CACHE_FOR_WRITE);
    if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
    }
    // store entry
    if (volume->m_fatType == 16) {
        pc->fat16[cluster & 0XFF] = value;
    }
    else {
        pc->fat32[cluster & 0X7F] = value;
    }
    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
// free a cluster chain
bool SdVolume_freeChain(SdVolume_t *volume, uint32_t cluster) {
    uint32_t next;

    do {
        if (!SdVolume_fatGet(volume, cluster, &next)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        // free cluster
        if (!SdVolume_fatPut(volume, cluster, 0)) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        if (cluster < volume->m_allocSearchStart)
            volume->m_allocSearchStart = cluster;
        cluster = next;
    } while (!SdVolume_isEOC(volume, cluster));

    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
/** Volume free space in clusters.
 *
 * \return Count of free clusters for success or -1 if an error occurs.
 */
int32_t SdVolume_freeClusterCount(SdVolume_t *volume) {
    uint32_t free = 0;
    uint32_t lba;
    uint32_t todo = volume->m_clusterCount + 2;
    uint16_t n;

    if (volume->m_fatType == 16 || volume->m_fatType == 32) {
        lba = volume->m_fatStartBlock;
        while (todo) {
            cache_t* pc = SdVolume_cacheFetchFat(volume, lba++, CACHE_FOR_READ);
            if (!pc) {
                DBG_FAIL_MACRO;
                goto fail;
            }
            n = volume->m_fatType == 16 ? 256 : 128;
            if (todo < n) n = todo;
            if (volume->m_fatType == 16) {
                for (uint16_t i = 0; i < n; i++) {
                    if (pc->fat16[i] == 0)
                        free++;
                }
            }
            else {
                for (uint16_t i = 0; i < n; i++) {
                    if (pc->fat32[i] == 0)
                        free++;
                }
            }
            todo -= n;
        }
    }
    else {
        // invalid FAT type
        DBG_FAIL_MACRO;
        goto fail;
    }
    return free;

    fail:
    return -1;
}
//------------------------------------------------------------------------------
/** Initialize a FAT volume.
 *
 * \param[in] dev The SD card where the volume is located.
 *
 * \param[in] part The partition to be used.  Legal values for \a part are
 * 1-4 to use the corresponding partition on a device formatted with
 * a MBR, Master Boot Record, or zero if the device is formatted as
 * a super floppy with the FAT boot sector in block zero.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  Reasons for
 * failure include not finding a valid partition, not finding a valid
 * FAT file system in the specified partition or an I/O error.
 */
bool SdVolume_init(SdVolume_t *volume, Sd2Card_t* dev, uint8_t part) {
    uint8_t tmp;
    uint32_t totalBlocks;
    uint32_t volumeStartBlock = 0;
    fat32_boot_t* fbs;
    cache_t* pc;
    volume->m_sdCard = dev;
    volume->m_fatType = 0;
    volume->m_allocSearchStart = 2;
    volume->m_cacheStatus = 0;  // cacheSync() will write block if true
    volume->m_cacheBlockNumber = 0XFFFFFFFF;
    volume->m_cacheFatStatus = 0;  // cacheSync() will write block if true
    volume->m_cacheFatBlockNumber = 0XFFFFFFFF;
    // if part == 0 assume super floppy with FAT boot sector in block zero
    // if part > 0 assume mbr volume with partition table
    if (part) {
        if (part > 4) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        pc = SdVolume_cacheFetch(volume, volumeStartBlock, CACHE_FOR_READ);
        if (!pc) {
            DBG_FAIL_MACRO;
            goto fail;
        }
        part_t* p = &pc->mbr.part[part-1];
        if ((p->boot & 0X7F) != 0 || p->firstSector == 0) {
            // not a valid partition
            DBG_FAIL_MACRO;
            goto fail;
        }
        volumeStartBlock = p->firstSector;
    }
    pc = SdVolume_cacheFetch(volume, volumeStartBlock, CACHE_FOR_READ);
    if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
    }
    fbs = &(pc->fbs32);
    if (fbs->bytesPerSector != 512 || fbs->fatCount == 0 || fbs->reservedSectorCount == 0) {
        // not valid FAT volume
        DBG_FAIL_MACRO;
        goto fail;
    }
    volume->m_fatCount = fbs->fatCount;
    volume->m_blocksPerCluster = fbs->sectorsPerCluster;
    // determine shift that is same as multiply by m_blocksPerCluster
    volume->m_clusterSizeShift = 0;
    for (tmp = 1; volume->m_blocksPerCluster != tmp; volume->m_clusterSizeShift++) {
        tmp <<= 1;
        if (tmp == 0) {
            DBG_FAIL_MACRO;
            goto fail;
        }
    }

    volume->m_blocksPerFat = fbs->sectorsPerFat16 ? fbs->sectorsPerFat16 : fbs->sectorsPerFat32;
    volume->m_fatStartBlock = volumeStartBlock + fbs->reservedSectorCount;

    // count for FAT16 zero for FAT32
    volume->m_rootDirEntryCount = fbs->rootDirEntryCount;

    // directory start for FAT16 dataStart for FAT32
    volume->m_rootDirStart = volume->m_fatStartBlock + fbs->fatCount * volume->m_blocksPerFat;

    // data start for FAT16 and FAT32
    volume->m_dataStartBlock = volume->m_rootDirStart + ((32 * fbs->rootDirEntryCount + 511)/512);

    // total blocks for FAT16 or FAT32
    totalBlocks = fbs->totalSectors16 ? fbs->totalSectors16 : fbs->totalSectors32;
    // total data blocks
    volume->m_clusterCount = totalBlocks - (volume->m_dataStartBlock - volumeStartBlock);

    // divide by cluster size to get cluster count
    volume->m_clusterCount >>= volume->m_clusterSizeShift;

    // FAT type is determined by cluster count
    if (volume->m_clusterCount < 4085) {
        volume->m_fatType = 12;
        //if (!FAT12_SUPPORT) {
            DBG_FAIL_MACRO;
            goto fail;
        //}
    }
    else if (volume->m_clusterCount < 65525) {
        volume->m_fatType = 16;
    }
    else {
        volume->m_rootDirStart = fbs->fat32RootCluster;
        volume->m_fatType = 32;
    }
    return true;

    fail:
    return false;
}
