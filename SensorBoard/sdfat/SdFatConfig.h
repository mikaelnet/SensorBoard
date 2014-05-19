/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __SDFATCONFIG_H
#define __SDFATCONFIG_H

#include <stdint.h>
//------------------------------------------------------------------------------
/**
 * Set USE_SEPARATE_FAT_CACHE nonzero to use a second 512 byte cache
 * for FAT table entries.  Improves performance for large writes that
 * are not a multiple of 512 bytes.
 */

#define USE_SEPARATE_FAT_CACHE 1

//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_BLOCK_SD_IO nonzero to use multi-block SD read/write.
 *
 * Don't use mult-block read/write on small AVR boards.
 */

#define USE_MULTI_BLOCK_SD_IO 1


//------------------------------------------------------------------------------
/**
 * To use multiple SD cards set USE_MULTIPLE_CARDS nonzero.
 *
 * Using multiple cards costs about 200  bytes of flash.
 *
 * Each card requires about 550 bytes of SRAM so use of a Mega is recommended.
 */
#define USE_MULTIPLE_CARDS 0


#endif  // SdFatConfig_h
