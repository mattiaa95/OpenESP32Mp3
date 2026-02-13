#ifndef PLAYBACK_CONTROL_H
#define PLAYBACK_CONTROL_H

#include <cstdint>

/* ============================================================================
 * Playback Control State Machine
 * Manages playback lifecycle: IDLE → LOADING → PLAYING → PAUSED → ERROR
 * ========================================================================== */

enum PlaybackState {
    STATE_IDLE,
    STATE_LOADING,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_ERROR
};

enum PlaybackCommand {
    CMD_PLAY_NEXT,
    CMD_PLAY_PREV,
    CMD_TOGGLE_PLAY_PAUSE,
    CMD_STOP,
    CMD_NONE
};

class PlaybackController {
public:
    virtual ~PlaybackController() = default;
    
    virtual bool init() = 0;
    virtual PlaybackState get_state() const = 0;
    virtual void execute_command(PlaybackCommand cmd) = 0;
    virtual void update() = 0;  /* Called periodically from main loop */
    virtual uint32_t get_current_position_ms() const = 0;
    virtual uint32_t get_total_duration_ms() const = 0;
};

PlaybackController* create_playback_controller();

#endif  // PLAYBACK_CONTROL_H
