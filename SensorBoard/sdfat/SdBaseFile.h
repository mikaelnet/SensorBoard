#if 0
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
#ifndef SdBaseFile_h
#define SdBaseFile_h
/**
 * \file
 * \brief SdBaseFile class
 */
#ifdef __AVR__
#include <avr/pgmspace.h>
#else  // __AVR__
#ifndef PGM_P
/** pointer to flash for ARM */
#define PGM_P const char*
#endif  // PGM_P
#ifndef PSTR
/** store literal string in flash for ARM */
#define PSTR(x) (x)
#endif  // PSTR
#ifndef pgm_read_byte
/** read 8-bits from flash for ARM */
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif  // pgm_read_byte
#ifndef pgm_read_word
/** read 16-bits from flash for ARM */
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif  // pgm_read_word
#ifndef PROGMEM
/** store in flash for ARM */
#define PROGMEM const
#endif  // PROGMEM
#endif  // __AVR__

#include "SdFatConfig.h"
#include "SdVolume.h"
//------------------------------------------------------------------------------
/**
 * \struct FatPos_t
 * \brief internal type for istream
 * do not use in user apps
 */
typedef struct FatPos_struct {
  /** stream position */
  uint32_t position;
  /** cluster for position */
  uint32_t cluster;
} FatPos_t;

// use the gnu style oflag in open()
/** open() oflag for reading */
uint8_t const O_READ = 0X01;
/** open() oflag - same as O_IN */
uint8_t const O_RDONLY = O_READ;
/** open() oflag for write */
uint8_t const O_WRITE = 0X02;
/** open() oflag - same as O_WRITE */
uint8_t const O_WRONLY = O_WRITE;
/** open() oflag for reading and writing */
uint8_t const O_RDWR = (O_READ | O_WRITE);
/** open() oflag mask for access modes */
uint8_t const O_ACCMODE = (O_READ | O_WRITE);
/** The file offset shall be set to the end of the file prior to each write. */
uint8_t const O_APPEND = 0X04;
/** synchronous writes - call sync() after each write */
uint8_t const O_SYNC = 0X08;
/** truncate the file to zero length */
uint8_t const O_TRUNC = 0X10;
/** set the initial position at the end of the file */
uint8_t const O_AT_END = 0X20;
/** create the file if nonexistent */
uint8_t const O_CREAT = 0X40;
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
uint8_t const O_EXCL = 0X80;

// SdBaseFile class static and const definitions
// flags for ls()
/** ls() flag to print modify date */
uint8_t const LS_DATE = 1;
/** ls() flag to print file size */
uint8_t const LS_SIZE = 2;
/** ls() flag for recursive list of subdirectories */
uint8_t const LS_R = 4;


// flags for timestamp
/** set the file's last access date */
uint8_t const T_ACCESS = 1;
/** set the file's creation date and time */
uint8_t const T_CREATE = 2;
/** Set the file's write date and time */
uint8_t const T_WRITE = 4;
// values for m_type
/** This file has not been opened. */
uint8_t const FAT_FILE_TYPE_CLOSED = 0;
/** A normal file */
uint8_t const FAT_FILE_TYPE_NORMAL = 1;
/** A FAT12 or FAT16 root directory */
uint8_t const FAT_FILE_TYPE_ROOT_FIXED = 2;
/** A FAT32 root directory */
uint8_t const FAT_FILE_TYPE_ROOT32 = 3;
/** A subdirectory file*/
uint8_t const FAT_FILE_TYPE_SUBDIR = 4;
/** Test value for directory type */
uint8_t const FAT_FILE_TYPE_MIN_DIR = FAT_FILE_TYPE_ROOT_FIXED;

/** date field for FAT directory entry
 * \param[in] year [1980,2107]
 * \param[in] month [1,12]
 * \param[in] day [1,31]
 *
 * \return Packed date for dir_t entry.
 */
static inline uint16_t FAT_DATE(uint16_t year, uint8_t month, uint8_t day) {
  return (year - 1980) << 9 | month << 5 | day;
}
/** year part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted year [1980,2107]
 */
static inline uint16_t FAT_YEAR(uint16_t fatDate) {
  return 1980 + (fatDate >> 9);
}
/** month part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted month [1,12]
 */
static inline uint8_t FAT_MONTH(uint16_t fatDate) {
  return (fatDate >> 5) & 0XF;
}
/** day part of FAT directory date field
 * \param[in] fatDate Date in packed dir format.
 *
 * \return Extracted day [1,31]
 */
static inline uint8_t FAT_DAY(uint16_t fatDate) {
  return fatDate & 0X1F;
}
/** time field for FAT directory entry
 * \param[in] hour [0,23]
 * \param[in] minute [0,59]
 * \param[in] second [0,59]
 *
 * \return Packed time for dir_t entry.
 */
static inline uint16_t FAT_TIME(uint8_t hour, uint8_t minute, uint8_t second) {
  return hour << 11 | minute << 5 | second >> 1;
}
/** hour part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted hour [0,23]
 */
static inline uint8_t FAT_HOUR(uint16_t fatTime) {
  return fatTime >> 11;
}
/** minute part of FAT directory time field
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted minute [0,59]
 */
static inline uint8_t FAT_MINUTE(uint16_t fatTime) {
  return(fatTime >> 5) & 0X3F;
}
/** second part of FAT directory time field
 * Note second/2 is stored in packed time.
 *
 * \param[in] fatTime Time in packed dir format.
 *
 * \return Extracted second [0,58]
 */
static inline uint8_t FAT_SECOND(uint16_t fatTime) {
  return 2*(fatTime & 0X1F);
}

/** Default date for file timestamps is 1 Jan 2000 */
uint16_t const FAT_DEFAULT_DATE = ((2000 - 1980) << 9) | (1 << 5) | 1;
/** Default time for file timestamp is 1 am */
uint16_t const FAT_DEFAULT_TIME = (1 << 11);
//------------------------------------------------------------------------------
/**
 * \class SdBaseFile
 * \brief Base class for SdFile with Print and C++ streams.
 */

  /** Create an instance. */
  //SdBaseFile() : writeError(false), m_type(FAT_FILE_TYPE_CLOSED) {}
  //SdBaseFile(const char* path, uint8_t oflag);

  /**
   * writeError is set to true if an error occurs during a write().
   * Set writeError to false before calling print() and/or write() and check
   * for true after calls to print() and/or write().
   */
bool writeError;
/** \return value of writeError */
bool SdFile_getWriteError() {return writeError;}
/** Set writeError to zero */
void SdFile_clearWriteError() {writeError = 0;}
//----------------------------------------------------------------------------
// helpers for stream classes
/** get position for streams
* \param[out] pos struct to receive position
*/
void SdFile_getpos(FatPos_t* pos);
/** set position for streams
* \param[out] pos struct with value for new position
*/
void SdFile_setpos(FatPos_t* pos);
//----------------------------------------------------------------------------
/** \return number of bytes available from yhe current position to EOF */
uint32_t SdFile_available() {return fileSize() - curPosition();}
bool SdFile_close();
bool SdFile_contiguousRange(uint32_t* bgnBlock, uint32_t* endBlock);
bool SdFile_createContiguous(SdBaseFile* dirFile,
        const char* path, uint32_t size);
/** \return The current cluster number for a file or directory. */
uint32_t SdFile_curCluster() const {return m_curCluster;}
/** \return The current position for a file or directory. */
uint32_t SdFile_curPosition() const {return m_curPosition;}
/** \return Current working directory */
static SdBaseFile* SdFile_cwd() {return m_cwd;}
/** Set the date/time callback function
*
* \param[in] dateTime The user's call back function.  The callback
* function is of the form:
*
* \code
* void dateTime(uint16_t* date, uint16_t* time) {
*   uint16_t year;
*   uint8_t month, day, hour, minute, second;
*
*   // User gets date and time from GPS or real-time clock here
*
*   // return date using FAT_DATE macro to format fields
*   *date = FAT_DATE(year, month, day);
*
*   // return time using FAT_TIME macro to format fields
*   *time = FAT_TIME(hour, minute, second);
* }
* \endcode
*
* Sets the function that is called when a file is created or when
* a file's directory entry is modified by sync(). All timestamps,
* access, creation, and modify, are set when a file is created.
* sync() maintains the last access date and last modify date/time.
*
* See the timestamp() function.
*/
static void dateTimeCallback(
void (*dateTime)(uint16_t* date, uint16_t* time)) {
m_dateTime = dateTime;
}
/**  Cancel the date/time callback function. */
static void SdFile_dateTimeCallbackCancel() {m_dateTime = 0;}
bool SdFile_dirEntry(dir_t* dir);
static void SdFile_dirName(const dir_t& dir, char* name);
bool SdFile_exists(const char* name);
int16_t SdFile_fgets(char* str, int16_t num, char* delim = 0);
/** \return The total number of bytes in a file or directory. */
uint32_t SdFile_fileSize() const {return m_fileSize;}
/** \return The first cluster number for a file or directory. */
uint32_t SdFile_firstCluster() const {return m_firstCluster;}
bool SdFile_getFilename(char* name);
/** \return True if this is a directory else false. */
bool SdFile_isDir() const {return m_type >= FAT_FILE_TYPE_MIN_DIR;}
/** \return True if this is a normal file else false. */
bool SdFile_isFile() const {return m_type == FAT_FILE_TYPE_NORMAL;}
/** \return True if this is an open file/directory else false. */
bool SdFile_isOpen() const {return m_type != FAT_FILE_TYPE_CLOSED;}
/** \return True if this is a subdirectory else false. */
bool SdFile_isSubDir() const {return m_type == FAT_FILE_TYPE_SUBDIR;}
/** \return True if this is the root directory. */
bool SdFile_isRoot() const {
return m_type == FAT_FILE_TYPE_ROOT_FIXED || m_type == FAT_FILE_TYPE_ROOT32;
}
void SdFile_ls(Print* pr, uint8_t flags = 0, uint8_t indent = 0);
void SdFile_ls(uint8_t flags = 0);
bool SdFile_mkdir(SdBaseFile* dir, const char* path, bool pFlag = true);
// alias for backward compactability
bool SdFile_makeDir(SdBaseFile* dir, const char* path) {
return mkdir(dir, path, false);
}
bool SdFile_open(SdBaseFile* dirFile, uint16_t index, uint8_t oflag);
bool SdFile_open(SdBaseFile* dirFile, const char* path, uint8_t oflag);
bool SdFile_open(const char* path, uint8_t oflag = O_READ);
bool SdFile_openNext(SdBaseFile* dirFile, uint8_t oflag);
bool SdFile_openRoot(SdVolume* vol);
int SdFile_peek();
bool SdFile_printCreateDateTime(Print* pr);
static void SdFile_printFatDate(uint16_t fatDate);
static void SdFile_printFatDate(Print* pr, uint16_t fatDate);
static void SdFile_printFatTime(uint16_t fatTime);
static void SdFile_printFatTime(Print* pr, uint16_t fatTime);
int SdFile_printField(int16_t value, char term);
int SdFile_printField(uint16_t value, char term);
int SdFile_printField(int32_t value, char term);
int SdFile_printField(uint32_t value, char term);
bool SdFile_printModifyDateTime(Print* pr);
bool SdFile_printName();
bool SdFile_printName(Print* pr);
int16_t SdFile_read();
int SdFile_read(void* buf, size_t nbyte);
int8_t SdFile_readDir(dir_t* dir);
static bool SdFile_remove(SdBaseFile* dirFile, const char* path);
bool SdFile_remove();
/** Set the file's current position to zero. */
void SdFile_rewind() {seekSet(0);}
bool SdFile_rename(SdBaseFile* dirFile, const char* newPath);
bool SdFile_rmdir();
// for backward compatibility
bool SdFile_rmDir() {return rmdir();}
bool SdFile_rmRfStar();
/** Set the files position to current position + \a pos. See seekSet().
* \param[in] offset The new position in bytes from the current position.
* \return true for success or false for failure.
*/
bool SdFile_seekCur(int32_t offset) {
return seekSet(m_curPosition + offset);
}
/** Set the files position to end-of-file + \a offset. See seekSet().
* \param[in] offset The new position in bytes from end-of-file.
* \return true for success or false for failure.
*/
bool SdFile_seekEnd(int32_t offset = 0) {return seekSet(m_fileSize + offset);}
bool SdFile_seekSet(uint32_t pos);
bool SdFile_sync();
bool SdFile_timestamp(SdBaseFile* file);
bool SdFile_timestamp(uint8_t flag, uint16_t year, uint8_t month, uint8_t day,
        uint8_t hour, uint8_t minute, uint8_t second);
/** Type of file.  You should use isFile() or isDir() instead of type()
* if possible.
*
* \return The file or directory type.
*/
uint8_t SdFile_type() const {return m_type;}
bool SdFile_truncate(uint32_t size);
/** \return SdVolume that contains this file. */
SdVolume* SdFile_volume() const {return m_vol;}
int SdFile_write(const void* buf, size_t nbyte);

//------------------------------------------------------------------------------
 private:
  // allow SdFat to set m_cwd
  friend class SdFat;
  /** experimental don't use */
  bool openParent(SdBaseFile* dir);

  // private functions
  bool addCluster();
  cache_t* addDirCluster();
  dir_t* cacheDirEntry(uint8_t action);
  int8_t lsPrintNext(Print *pr, uint8_t flags, uint8_t indent);
  static bool make83Name(const char* str, uint8_t* name, const char** ptr);
  bool mkdir(SdBaseFile* parent, const uint8_t dname[11]);
  bool open(SdBaseFile* dirFile, const uint8_t dname[11], uint8_t oflag);
  bool openCachedEntry(uint8_t cacheIndex, uint8_t oflags);
  dir_t* readDirCache();
  static void setCwd(SdBaseFile* cwd) {m_cwd = cwd;}
  bool setDirSize();

  // bits defined in m_flags
  // should be 0X0F
  static uint8_t const F_OFLAG = (O_ACCMODE | O_APPEND | O_SYNC);
  // sync of directory entry required
  static uint8_t const F_FILE_DIR_DIRTY = 0X80;

  // global pointer to cwd dir
  static SdBaseFile* m_cwd;
  // data time callback function
  static void (*m_dateTime)(uint16_t* date, uint16_t* time);
  // private data
  uint8_t   m_flags;         // See above for definition of m_flags bits
  uint8_t   m_type;          // type of file see above for values
  uint8_t   m_dirIndex;      // index of directory entry in dirBlock
  SdVolume* m_vol;           // volume where file is located
  uint32_t  m_curCluster;    // cluster for current file position
  uint32_t  m_curPosition;   // current file position in bytes from beginning
  uint32_t  m_dirBlock;      // block for this files directory entry
  uint32_t  m_fileSize;      // file size in bytes
  uint32_t  m_firstCluster;  // first cluster of file
};
#endif  // SdBaseFile_h

#endif