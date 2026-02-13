#include "sd_card.h"
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <cstring>

#include "config.h"

class SDCardImpl : public SDCard {
private:
    bool mounted = false;
    File current_file;
    
public:
    bool init() override;
    bool is_mounted() const override;
    int list_files(const char** filenames, int max_count) override;
    bool open_file(const char* filename) override;
    int read_data(uint8_t* buffer, size_t max_len) override;
    void close_file() override;
    size_t get_file_size() const override;
    void unmount() override;
    const char* get_error_message() const override;
};

bool SDCardImpl::init() {
    // Initialize SPI with conservative clock speed
    SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_CS_PIN);
    
    // Initialize SD card with CS pin
    if (!SD.begin(SPI_CS_PIN, SPI, SPI_CLOCK_FREQ * 1000000)) {
        Serial.println("[SD] Failed to initialize SD card");
        return false;
    }
    
    mounted = true;
    Serial.println("[SD] SD card initialized successfully");
    return true;
}

bool SDCardImpl::is_mounted() const {
    return mounted;
}

int SDCardImpl::list_files(const char** filenames, int max_count) {
    if (!mounted) {
        Serial.println("[SD] Card not mounted");
        return 0;
    }
    
    File root = SD.open("/");
    if (!root) {
        Serial.println("[SD] Failed to open root directory");
        return 0;
    }
    
    int count = 0;
    while (count < max_count) {
        File entry = root.openNextFile();
        if (!entry) {
            break;
        }
        
        if (!entry.isDirectory()) {
            // Check if MP3 file
            const char* name = entry.name();
            if (strlen(name) > 4) {
                const char* ext = name + strlen(name) - 4;
                if (strcasecmp(ext, ".mp3") == 0) {
                    filenames[count] = name;
                    count++;
                    Serial.printf("[SD] Found MP3: %s (%d bytes)\n", name, entry.size());
                }
            }
        }
        entry.close();
    }
    
    root.close();
    Serial.printf("[SD] Listed %d MP3 files\n", count);
    return count;
}

bool SDCardImpl::open_file(const char* filename) {
    if (!mounted) {
        Serial.println("[SD] Card not mounted");
        return false;
    }
    
    close_file();  // Close any open file first
    current_file = SD.open(filename, FILE_READ);
    
    if (!current_file) {
        Serial.printf("[SD] Failed to open file: %s\n", filename);
        return false;
    }
    
    Serial.printf("[SD] Opened file: %s (size: %d bytes)\n", filename, current_file.size());
    return true;
}

int SDCardImpl::read_data(uint8_t* buffer, size_t max_len) {
    if (!current_file) {
        return -1;
    }
    
    int bytes_read = current_file.read(buffer, max_len);
    if (bytes_read < 0) {
        Serial.println("[SD] Error reading file");
        return -1;
    }
    
    return bytes_read;
}

void SDCardImpl::close_file() {
    if (current_file) {
        current_file.close();
    }
}

size_t SDCardImpl::get_file_size() const {
    if (!current_file) {
        return 0;
    }
    return current_file.size();
}

void SDCardImpl::unmount() {
    close_file();
    SD.end();
    mounted = false;
    Serial.println("[SD] SD card unmounted");
}

const char* SDCardImpl::get_error_message() const {
    return "SD card error";
}

// Global singleton
static SDCardImpl g_sd_card;

SDCard* create_sd_card() {
    return &g_sd_card;
}
