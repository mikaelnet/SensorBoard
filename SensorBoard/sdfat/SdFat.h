/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SDFAT_H
#define SDFAT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * \file
 * \brief SdFat class
 */
//------------------------------------------------------------------------------
/** Macro for debug. */
#define DBG_FAIL_MACRO  // printf_P(PSTR("Fail: %s:%s\n"), __FILE__, __LINE__);
//------------------------------------------------------------------------------

/** SdFat version YYYYMMDD */
#define SD_FAT_VERSION 20131225

//------------------------------------------------------------------------------
#include "SdFile.h"
//#include "SdStream.h"

#if 0

typedef struct SdFat_struct {
    Sd2Card_t m_card;
    FILE *m_stdOut;
    SdVolume_t m_vol;
    SdBaseFile_t m_vwd;
} SdFat_t;


/** \return a pointer to the Sd2Card object. */
Sd2Card* SdFat_card(SdFat_t *sdfat) {return &m_card;}
bool SdFat_chdir(SdFat_t *sdfat, const char* path, bool set_cwd);
void SdFat_chvol(SdFat_t *sdfat);
void SdFat_errorHalt(SdFat_t *sdfat);
void SdFat_errorHaltMsg(SdFat_t *sdfat, char const *msg);
void SdFat_errorPrint(SdFat_t *sdfat);
void SdFat_errorPrintMsg(SdFat_t *sdfat, char const *msg);
bool SdFat_exists(SdFat_t *sdfat, const char* name);
bool SdFat_begin(SdFat_t *sdfat);
void SdFat_initErrorHalt(SdFat_t *sdfat);
void SdFat_initErrorHaltMsg(SdFat_t *sdfat, char const *msg);
void SdFat_initErrorPrint(SdFat_t *sdfat);
void SdFat_initErrorPrintMsg(SdFat_t *sdfat, char const *msg);
void SdFat_ls(SdFat_t *sdfat);
bool SdFat_mkdir(SdFat_t *sdfat, const char* path, bool pFlag);
bool SdFat_remove(SdFat_t *sdfat, const char* path);
bool SdFat_rename(SdFat_t *sdfat, const char *oldPath, const char *newPath);
bool SdFat_rmdir(SdFat_t *sdfat, const char* path);
bool SdFat_truncate(SdFat_t *sdfat, const char* path, uint32_t length);
/** \return a pointer to the SdVolume object. */
SdVolume* SdFat_vol(SdFat_t *sdfat) {return &m_vol;}
/** \return a pointer to the volume working directory. */
SdBaseFile* SdFat_vwd(SdFat_t *sdfat) {return &m_vwd;}
//----------------------------------------------------------------------------
void SdFat_errorHalt_P(SdFat_t *sdfat, PGM_P msg);
void SdFat_errorPrint_P(SdFat_t *sdfat, PGM_P msg);
void SdFat_initErrorHalt_P(SdFat_t *sdfat, PGM_P msg);
void SdFat_initErrorPrint_P(SdFat_t *sdfat, PGM_P msg);
//----------------------------------------------------------------------------
/**
*  Set stdOut Print stream for messages.
* \param[in] stream The new Print stream.
*/
static void setStdOut(Print* stream) {m_stdOut = stream;}
/** \return Print stream for messages. */
static Print* stdOut() {return m_stdOut;}


#endif  // SdFat_h
#endif