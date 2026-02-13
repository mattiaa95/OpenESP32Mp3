#include "sd_card.h"

class SDCardImpl : public SDCard {
private:
    bool mounted = false;
    
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
    // TODO: Initialize SPI and mount FAT32
    return false;
}

bool SDCardImpl::is_mounted() const {
    return mounted;
}

int SDCardImpl::list_files(const char** filenames, int max_count) {
    // TODO: Enumerate MP3 files
    return 0;
}

bool SDCardImpl::open_file(const char* filename) {
    // TODO: Open file for reading
    return false;
}

int SDCardImpl::read_data(uint8_t* buffer, size_t max_len) {
    // TODO: Read file data
    return 0;
}

void SDCardImpl::close_file() {
    // TODO: Close file
}

size_t SDCardImpl::get_file_size() const {
    return 0;
}

void SDCardImpl::unmount() {
    // TODO: Unmount FAT32
    mounted = false;
}

const char* SDCardImpl::get_error_message() const {
    return "Not implemented";
}
