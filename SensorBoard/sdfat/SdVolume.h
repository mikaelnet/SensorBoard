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
#ifndef SdVolume_h
#define SdVolume_h
/**
 * \file
 * \brief SdVolume class
 */
#include "SdFatConfig.h"
#include "Sd2Card.h"
#include "SdFatStructs.h"

#include <stdbool.h>
#include <stdint.h>

//==============================================================================
// SdVolume class
/**
 * \brief Cache for an SD data block
 */
typedef union cache_union {
    /** Used to access cached file data blocks. */
    uint8_t  data[512];
    /** Used to access cached FAT16 entries. */
    uint16_t fat16[256];
    /** Used to access cached FAT32 entries. */
    uint32_t fat32[128];
    /** Used to access cached directory entries. */
    dir_t    dir[16];
    /** Used to access a cached Master Boot Record. */
    mbr_t    mbr;
    /** Used to access to a cached FAT boot sector. */
    fat_boot_t fbs;
    /** Used to access to a cached FAT32 boot sector. */
    fat32_boot_t fbs32;
    /** Used to access to a cached FAT32 FSINFO sector. */
    fat32_fsinfo_t fsinfo;
} cache_t;

typedef struct SdVolume_struct {
    uint32_t m_allocSearchStart;   // Start cluster for alloc search.
    uint8_t m_blocksPerCluster;    // Cluster size in blocks.
    uint32_t m_clusterCount;       // Clusters in one FAT.
    uint8_t m_clusterSizeShift;    // Cluster count to block count shift.
    uint32_t m_dataStartBlock;     // First data block number.
    uint32_t m_fatStartBlock;      // Start block for first FAT.
    uint8_t m_fatType;             // Volume type (12, 16, OR 32).
    uint16_t m_rootDirEntryCount;  // Number of entries in FAT16 root dir.
    uint32_t m_rootDirStart;       // Start block for FAT16, cluster for FAT32.

    uint8_t m_fatCount;           // number of FATs on volume
    uint32_t m_blocksPerFat;      // FAT size in blocks
    cache_t m_cacheBuffer;        // 512 byte cache for device blocks
    uint32_t m_cacheBlockNumber;  // Logical number of block in the cache
    Sd2Card_t* m_sdCard;            // Sd2Card object for cache
    uint8_t m_cacheStatus;        // status of cache block
    cache_t m_cacheFatBuffer;       // 512 byte cache for FAT
    uint32_t m_cacheFatBlockNumber;  // current Fat block number
    uint8_t  m_cacheFatStatus;       // status of cache Fatblock
} SdVolume_t;

//------------------------------------------------------------------------------
/**
 * \class SdVolume
 * \brief Access FAT16 and FAT32 volumes on SD and SDHC cards.
 */

/** Clear the cache and returns a pointer to the cache.  Used by the WaveRP
* recorder to do raw write to the SD card.  Not for normal apps.
* \return A pointer to the cache buffer or zero if an error occurs.
*/
cache_t* SdVolume_cacheClear(SdVolume_t *volume);

/** Initialize a FAT volume.  Try partition one first then try super
   * floppy format.
   *
   * \param[in] dev The Sd2Card where the volume is located.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.  Reasons for
   * failure include not finding a valid partition, not finding a valid
   * FAT file system or an I/O error.
   */
bool SdVolume_init(SdVolume_t *volume, Sd2Card_t* dev, uint8_t part);

// inline functions that return volume info
/** \return The volume's cluster size in blocks. */
uint8_t SdVolume_blocksPerCluster(SdVolume_t *volume);
/** \return The number of blocks in one FAT. */
uint32_t SdVolume_blocksPerFat(SdVolume_t *volume);
/** \return The total number of clusters in the volume. */
uint32_t SdVolume_clusterCount(SdVolume_t *volume);
/** \return The shift count required to multiply by blocksPerCluster. */
uint8_t SdVolume_clusterSizeShift(SdVolume_t *volume);
/** \return The logical block number for the start of file data. */
uint32_t SdVolume_dataStartBlock(SdVolume_t *volume);
/** \return The number of FAT structures on the volume. */
uint8_t SdVolume_fatCount(SdVolume_t *volume);
/** \return The logical block number for the start of the first FAT. */
uint32_t SdVolume_fatStartBlock(SdVolume_t *volume);
/** \return The FAT type of the volume. Values are 12, 16 or 32. */
uint8_t SdVolume_fatType(SdVolume_t *volume);
int32_t SdVolume_freeClusterCount(SdVolume_t *volume);
/** \return The number of entries in the root directory for FAT16 volumes. */
uint32_t SdVolume_rootDirEntryCount(SdVolume_t *volume);
/** \return The logical block number for the start of the root directory
    on FAT16 volumes or the first cluster number on FAT32 volumes. */
uint32_t SdVolume_rootDirStart(SdVolume_t *volume);
/** Sd2Card object for this volume
* \return pointer to Sd2Card object.
*/
Sd2Card_t* SdVolume_sdCard(SdVolume_t *volume);

/** Debug access to FAT table
*
* \param[in] n cluster number.
* \param[out] v value of entry
* \return true for success or false for failure
*/
bool SdVolume_dbgFat(SdVolume_t *volume, uint32_t n, uint32_t* v);

// Friend with SdFile
bool SdVolume_allocContiguous(SdVolume_t *volume, uint32_t count, uint32_t* curCluster);
uint8_t SdVolume_blockOfCluster(SdVolume_t *volume, uint32_t position);
cache_t *SdVolume_cacheAddress(SdVolume_t *volume);
uint32_t SdVolume_cacheBlockNumber(SdVolume_t *volume);
void SdVolume_cacheInvalidate(SdVolume_t *volume);
uint32_t SdVolume_clusterStartBlock(SdVolume_t *volume, uint32_t cluster);
bool SdVolume_freeChain(SdVolume_t *volume, uint32_t cluster);
bool SdVolume_readBlock(SdVolume_t *volume, uint32_t block, uint8_t* dst);
bool SdVolume_writeBlock(SdVolume_t *volume, uint32_t block, const uint8_t* dst);

#endif  // SdVolume
