#include "audio_decoder.h"
#include "config.h"
#include "sd_card.h"
#include <cstring>
#include <Arduino.h>

/* ============================================================================
 * MP3 Decoder Implementation
 * Handles MP3 frame parsing, decoding, and PCM output
 * Uses frame sync detection + dummy decode (libhelix to be integrated)
 * ========================================================================== */

class MP3Decoder : public AudioDecoder {
private:
    bool is_open = false;
    uint32_t duration_ms = 0;
    uint32_t current_pos_ms = 0;
    uint32_t total_frames = 0;
    
    int sample_rate = 0;
    int channels = 0;
    int bitrate = 0;
    
    uint8_t frame_buffer[2048];
    size_t frame_buffer_len = 0;
    
    bool find_frame_sync(const uint8_t* data, size_t len, size_t& sync_pos);
    bool parse_frame_header(const uint8_t* header);
    
public:
    bool open(const char* filepath) override;
    int decode_frame(int16_t* pcm_buffer, size_t max_samples) override;
    void close() override;
    uint32_t get_duration_ms() const override;
    uint32_t get_current_position_ms() const override;
    const char* get_error_message() const override;
};

bool MP3Decoder::find_frame_sync(const uint8_t* data, size_t len, size_t& sync_pos) {
    /* MP3 frame sync: 11 consecutive '1' bits (0xFFF pattern) */
    for (size_t i = 0; i < len - 1; i++) {
        if (data[i] == 0xFF && (data[i + 1] & 0xE0) == 0xE0) {
            sync_pos = i;
            return true;
        }
    }
    return false;
}

bool MP3Decoder::parse_frame_header(const uint8_t* header) {
    if (!header) return false;
    
    uint8_t h1 = header[1];
    uint8_t h2 = header[2];
    
    /* MPEG version (bits 3-4 of h1): 11=v1, 10=v2, 00=v2.5 */
    int version = (h1 >> 3) & 0x03;
    
    /* Sample rate index (bits 2-3 of h2) */
    int sr_idx = (h2 >> 2) & 0x03;
    static const int sample_rates_v1[] = {44100, 48000, 32000, 0};
    static const int sample_rates_v2[] = {22050, 24000, 16000, 0};
    
    if (version == 3) {
        sample_rate = sample_rates_v1[sr_idx];
    } else if (version == 2) {
        sample_rate = sample_rates_v2[sr_idx];
    } else {
        return false;  /* Invalid version */
    }
    
    if (sample_rate == 0) return false;
    
    /* Channels: bits 6-7 of h1 */
    int mode = (h1 >> 6) & 0x03;
    channels = (mode == 3) ? 1 : 2;
    
    /* Bitrate index (bits 4-7 of h1) */
    int br_idx = (h1 >> 4) & 0x0F;
    if (br_idx > 0 && br_idx < 15) {
        static const int bitrates[] = {
            0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
        };
        bitrate = bitrates[br_idx] * 1000;
    } else {
        return false;
    }
    
    return true;
}

bool MP3Decoder::open(const char* filepath) {
    if (!filepath) return false;
    
    is_open = true;
    frame_buffer_len = 0;
    sample_rate = 0;
    channels = 0;
    bitrate = 0;
    duration_ms = 0;
    current_pos_ms = 0;
    total_frames = 0;
    
    Serial.printf("[MP3] Opened file: %s\n", filepath);
    return true;
}

int MP3Decoder::decode_frame(int16_t* pcm_buffer, size_t max_samples) {
    if (!is_open || !pcm_buffer) {
        return -1;
    }
    
    /* Placeholder decode: return silence for now */
    /* Real implementation will integrate libhelix MP3Decode */
    
    if (total_frames == 0) {
        /* First frame: parse header and estimate duration */
        if (frame_buffer_len < 4) {
            return 0;  /* Need more data */
        }
        
        if (!parse_frame_header(frame_buffer)) {
            return -1;  /* Invalid frame */
        }
        
        Serial.printf("[MP3] Frame info: %d Hz, %d ch, %d kbps\n",
                     sample_rate, channels, bitrate/1000);
    }
    
    /* Generate silence PCM output (1152 samples typical MP3 frame) */
    size_t samples_per_frame = 1152;
    size_t out_samples = (max_samples < samples_per_frame) ? max_samples : samples_per_frame;
    
    memset(pcm_buffer, 0, out_samples * sizeof(int16_t));
    
    /* Update tracking */
    total_frames++;
    current_pos_ms += (samples_per_frame * 1000) / sample_rate;
    
    return (int)out_samples;
}

void MP3Decoder::close() {
    is_open = false;
    frame_buffer_len = 0;
    Serial.println("[MP3] Decoder closed");
}

uint32_t MP3Decoder::get_duration_ms() const {
    return duration_ms;
}

uint32_t MP3Decoder::get_current_position_ms() const {
    return current_pos_ms;
}

const char* MP3Decoder::get_error_message() const {
    return "MP3 decoder error";
}

/* Global singleton */
static MP3Decoder g_decoder;

AudioDecoder* create_audio_decoder() {
    return &g_decoder;
}
