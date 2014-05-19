/* AVR SdFat Library
 *
 * This is a fork from William Greiman Arduino SdFat Library
 * rewritten for Atmel AVR family Ansi-C.
 * GNU General Public License v3
 * <http://www.gnu.org/licenses/>.
 */
#include "SdFat.h"

//------------------------------------------------------------------------------
/**
 * Initialize an SdFat object.
 *
 * Initializes the SD card, SD volume, and root directory.
 *
 * \param[in] chipSelectPin SD chip select pin. See Sd2Card::init().
 * \param[in] sckDivisor value for SPI SCK divisor. See Sd2Card::init().
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_begin(SdFat_t *sdfat, uint8_t chipSelectPin, uint8_t sckDivisor) {
    return m_card.begin(chipSelectPin, sckDivisor)
         && m_vol.init(&m_card) && chdir(1);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory to root
 *
 * Changes the volume's working directory to the SD's root directory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_chdir(SdFat_t *sdfat, bool set_cwd) {
    if (set_cwd)
        SdBaseFile_setCwd(&m_vwd);
    if (m_vwd.isOpen())
        m_vwd.close();
    return m_vwd.openRoot(&m_vol);
}
//------------------------------------------------------------------------------
/** Change a volume's working directory
 *
 * Changes the volume working directory to the \a path subdirectory.
 * Optionally set the current working directory to the volume's
 * working directory.
 *
 * Example: If the volume's working directory is "/DIR", chdir("SUB")
 * will change the volume's working directory from "/DIR" to "/DIR/SUB".
 *
 * If path is "/", the volume's working directory will be changed to the
 * root directory
 *
 * \param[in] path The name of the subdirectory.
 *
 * \param[in] set_cwd Set the current working directory to this volume's
 *  working directory if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_chdir(SdFat_t *sdfat, const char *path, bool set_cwd) {
    SdBaseFile dir;
    if (path[0] == '/' && path[1] == '\0')
        return SdFat_chdir(sdfat, set_cwd);

    if (!dir.open(&m_vwd, path, O_READ))
        goto fail;
    if (!dir.isDir())
        goto fail;
    m_vwd = dir;
    if (set_cwd)
        SdBaseFile_setCwd(&m_vwd);
    return true;

    fail:
    return false;
}
//------------------------------------------------------------------------------
/** Set the current working directory to a volume's working directory.
 *
 * This is useful with multiple SD cards.
 *
 * The current working directory is changed to this volume's working directory.
 *
 * This is like the Windows/DOS \<drive letter>: command.
 */
void SdFat_chvol(SdFat_t *sdfat) {
    SdBaseFile_setCwd(&sdfat->m_vwd);
}
//------------------------------------------------------------------------------
/**
 * Test for the existence of a file.
 *
 * \param[in] name Name of the file to be tested for.
 *
 * \return true if the file exists else false.
 */
bool SdFat_exists(SdFat_t *sdfat, const char* name) {
    return SdBaseFile_exists(&sdfat->m_vwd, name);
}
//------------------------------------------------------------------------------
/** List the directory contents of the volume working directory to stdOut.
 *
 * \param[in] flags The inclusive OR of
 *
 * LS_DATE - %Print file modification date
 *
 * LS_SIZE - %Print file size.
 *
 * LS_R - Recursive list of subdirectories.
 */
void SdFat_ls(SdFat_t *sdfat, uint8_t flags) {
    SdBaseFile_ls(&sdfat->m_vwd, sdfat->m_stdOut, flags);
}


//------------------------------------------------------------------------------
/** Make a subdirectory in the volume working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * \param[in] pFlag Create missing parent directories if true.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_mkdir(SdFat_t *sdfat, const char* path, bool pFlag) {
    SdBaseFile sub;
    return sub.mkdir(&m_vwd, path, pFlag);
}
//------------------------------------------------------------------------------
/** Remove a file from the volume working directory.
*
* \param[in] path A path with a valid 8.3 DOS name for the file.
*
* \return The value one, true, is returned for success and
* the value zero, false, is returned for failure.
*/
bool SdFat_remove(SdFat_t *sdfat, const char* path) {
    return SdBaseFile_remove(&m_vwd, path);
}
//------------------------------------------------------------------------------
/** Rename a file or subdirectory.
 *
 * \param[in] oldPath Path name to the file or subdirectory to be renamed.
 *
 * \param[in] newPath New path name of the file or subdirectory.
 *
 * The \a newPath object must not exist before the rename call.
 *
 * The file to be renamed must not be open.  The directory entry may be
 * moved and file system corruption could occur if the file is accessed by
 * a file object that was opened before the rename() call.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_rename(SdFat_t *sdfat, const char *oldPath, const char *newPath) {
    SdBaseFile file;
    if (!file.open(oldPath, O_READ))
        return false;
    return file.rename(&m_vwd, newPath);
}
//------------------------------------------------------------------------------
/** Remove a subdirectory from the volume's working directory.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the subdirectory.
 *
 * The subdirectory file will be removed only if it is empty.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool SdFat_rmdir(SdFat_t *sdfat, const char* path) {
    SdBaseFile sub;
    if (!sub.open(path, O_READ)) return false;
    return sub.rmdir();
}
//------------------------------------------------------------------------------
/** Truncate a file to a specified length.  The current file position
 * will be maintained if it is less than or equal to \a length otherwise
 * it will be set to end of file.
 *
 * \param[in] path A path with a valid 8.3 DOS name for the file.
 * \param[in] length The desired length for the file.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 * Reasons for failure include file is read only, file is a directory,
 * \a length is greater than the current file size or an I/O error occurs.
 */
bool SdFat_truncate(SdFat_t *sdfat, const char* path, uint32_t length) {
    SdBaseFile file;
    if (!file.open(path, O_WRITE))
        return false;
    return file.truncate(length);
}
