#if 0
/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * \brief SdFile class
 */
#ifndef SdFile_h
#define SdFile_h

#include "SdBaseFile.h"

//------------------------------------------------------------------------------
/**
 * \class SdFile
 * \brief SdBaseFile with Print.
 */

/** \return value of writeError */
bool SdFile_getWriteError() {return SdBaseFile::getWriteError();}
/** Set writeError to zero */
void SdFile_clearWriteError() {SdBaseFile::clearWriteError();}
size_t SdFile_write(uint8_t b);
int SdFile_writeString(const char* str);
int SdFile_writeBlock(const void* buf, size_t nbyte);
void SdFile_write_P(PGM_P str);
void SdFile_writeln_P(PGM_P str);

#endif  // SdFile_h

#endif