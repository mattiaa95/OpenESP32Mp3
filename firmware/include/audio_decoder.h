#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * Audio Decoder Interface (Pure Virtual)
 * Abstracts MP3 decoding; implementations decode frames to PCM
 * ========================================================================== */

class AudioDecoder {
public:
    virtual ~AudioDecoder() = default;
    
    /* Open MP3 file from filesystem */
    virtual bool open(const char* filepath) = 0;
    
    /* Decode one frame; returns samples in pcm_buffer */
    virtual int decode_frame(int16_t* pcm_buffer, size_t max_samples) = 0;
    
    /* Close and cleanup resources */
    virtual void close() = 0;
    
    /* Get total duration in milliseconds (if known) */
    virtual uint32_t get_duration_ms() const = 0;
    
    /* Get current playback position in milliseconds */
    virtual uint32_t get_current_position_ms() const = 0;
    
    /* Seek to position (optional; default no-op) */
    virtual bool seek(uint32_t position_ms) { return false; }
    
    /* Error status / diagnostics */
    virtual const char* get_error_message() const = 0;
};

#endif  // AUDIO_DECODER_H
