#ifndef UI_H
#define UI_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * Flipper-Style UI Manager
 * Minimal monochrome interface for MP3 player on 128x64 OLED
 * ========================================================================== */

enum PlayerState {
    UI_STATE_IDLE,
    UI_STATE_PLAYING,
    UI_STATE_PAUSED,
    UI_STATE_LOADING,
    UI_STATE_ERROR
};

class UI {
public:
    virtual ~UI() = default;
    
    /* Initialize UI resources */
    virtual bool init() = 0;
    
    /* Update display based on player state */
    virtual void update_track_info(const char* title, const char* artist) = 0;
    virtual void update_playback_state(PlayerState state) = 0;
    virtual void update_progress(uint32_t current_ms, uint32_t duration_ms) = 0;
    virtual void update_volume(uint8_t volume) = 0;
    
    /* Render frame and update display */
    virtual void render() = 0;
    
    /* Show error message temporarily */
    virtual void show_error(const char* message) = 0;
    
    /* Show info message (e.g., "Bluetooth Connected") */
    virtual void show_info(const char* message) = 0;
};

/* Factory function */
UI* create_ui();

#endif  // UI_H
