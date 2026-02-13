#ifndef SD_CARD_H
#define SD_CARD_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * SD Card Interface (Pure Virtual)
 * SPI-based SD card access with FAT32 file system support
 * ========================================================================== */

class SDCard {
public:
    virtual ~SDCard() = default;
    
    /* Initialize SPI and mount SD card */
    virtual bool init() = 0;
    
    /* Check if SD card is present and accessible */
    virtual bool is_mounted() const = 0;
    
    /* Get list of MP3 files in root directory */
    virtual int list_files(const char** filenames, int max_count) = 0;
    
    /* Open file for reading */
    virtual bool open_file(const char* filename) = 0;
    
    /* Read next chunk of data */
    virtual int read_data(uint8_t* buffer, size_t max_len) = 0;
    
    /* Close file */
    virtual void close_file() = 0;
    
    /* Get file size in bytes */
    virtual size_t get_file_size() const = 0;
    
    /* Unmount SD card */
    virtual void unmount() = 0;
    
    /* Error status */
    virtual const char* get_error_message() const = 0;
};

#endif  // SD_CARD_H
