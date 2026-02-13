#include "audio_decoder.h"

class MP3Decoder : public AudioDecoder {
private:
    bool is_open = false;
    uint32_t duration_ms = 0;
    uint32_t current_pos_ms = 0;
    
public:
    bool open(const char* filepath) override;
    int decode_frame(int16_t* pcm_buffer, size_t max_samples) override;
    void close() override;
    uint32_t get_duration_ms() const override;
    uint32_t get_current_position_ms() const override;
    const char* get_error_message() const override;
};

bool MP3Decoder::open(const char* filepath) {
    // TODO: Implement libhelix initialization
    return false;
}

int MP3Decoder::decode_frame(int16_t* pcm_buffer, size_t max_samples) {
    // TODO: Implement frame decode
    return 0;
}

void MP3Decoder::close() {
    // TODO: Cleanup
    is_open = false;
}

uint32_t MP3Decoder::get_duration_ms() const {
    return duration_ms;
}

uint32_t MP3Decoder::get_current_position_ms() const {
    return current_pos_ms;
}

const char* MP3Decoder::get_error_message() const {
    return "Not implemented";
}
