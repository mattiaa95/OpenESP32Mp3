#include "playback_control.h"
#include "audio_decoder.h"
#include "bluetooth_a2dp.h"
#include "sd_card.h"
#include "ui.h"
#include <Arduino.h>

/* ============================================================================
 * Playback Control Implementation
 * Manages state machine: IDLE → LOADING → PLAYING → PAUSED → ERROR
 * ========================================================================== */

class PlaybackControllerImpl : public PlaybackController {
private:
    PlaybackState state = STATE_IDLE;
    PlaybackCommand pending_cmd = CMD_NONE;
    
    uint32_t current_position_ms = 0;
    uint32_t total_duration_ms = 0;
    
    const char* current_file = nullptr;
    int current_file_index = 0;
    
    /* Module references (obtained at runtime) */
    AudioDecoder* decoder = nullptr;
    BluetoothA2DP* bt = nullptr;
    SDCard* sd = nullptr;
    UI* ui = nullptr;
    
    void transition_to(PlaybackState new_state);
    void handle_command();
    void update_playback();
    
public:
    bool init() override;
    PlaybackState get_state() const override;
    void execute_command(PlaybackCommand cmd) override;
    void update() override;
    uint32_t get_current_position_ms() const override;
    uint32_t get_total_duration_ms() const override;
};

void PlaybackControllerImpl::transition_to(PlaybackState new_state) {
    if (new_state == state) return;
    
    const char* state_names[] = {"IDLE", "LOADING", "PLAYING", "PAUSED", "ERROR"};
    Serial.printf("[PLAYBACK] State: %s → %s\n", state_names[state], state_names[new_state]);
    
    state = new_state;
    
    if (ui) {
        ui->update_playback_state((PlayerState)state);
        ui->render();
    }
}

void PlaybackControllerImpl::handle_command() {
    if (pending_cmd == CMD_NONE) return;
    
    Serial.printf("[PLAYBACK] Command: %d (state=%d)\n", pending_cmd, state);
    
    switch (pending_cmd) {
        case CMD_PLAY_NEXT:
            if (state == STATE_PLAYING || state == STATE_PAUSED) {
                transition_to(STATE_LOADING);
                current_file_index++;
            }
            break;
            
        case CMD_PLAY_PREV:
            if (state == STATE_PLAYING || state == STATE_PAUSED) {
                transition_to(STATE_LOADING);
                if (current_file_index > 0) current_file_index--;
            }
            break;
            
        case CMD_TOGGLE_PLAY_PAUSE:
            if (state == STATE_PLAYING) {
                transition_to(STATE_PAUSED);
            } else if (state == STATE_PAUSED || state == STATE_IDLE) {
                transition_to(STATE_LOADING);
            }
            break;
            
        case CMD_STOP:
            transition_to(STATE_IDLE);
            current_position_ms = 0;
            if (decoder) decoder->close();
            break;
            
        case CMD_NONE:
        default:
            break;
    }
    
    pending_cmd = CMD_NONE;
}

void PlaybackControllerImpl::update_playback() {
    if (state == STATE_IDLE || state == STATE_PAUSED) {
        return;
    }
    
    if (state == STATE_LOADING) {
        /* Try to load next file */
        if (sd && decoder) {
            // Try to open file at index (placeholder)
            transition_to(STATE_PLAYING);
            Serial.printf("[PLAYBACK] Loaded file index %d\n", current_file_index);
        }
        return;
    }
    
    if (state == STATE_PLAYING) {
        /* Simulate playback progress (in reality, decoder updates position) */
        current_position_ms += 50;  /* ~50 ms per update call */
        
        if (current_position_ms > total_duration_ms && total_duration_ms > 0) {
            Serial.println("[PLAYBACK] Track ended, playing next");
            pending_cmd = CMD_PLAY_NEXT;
            transition_to(STATE_LOADING);
        }
    }
}

bool PlaybackControllerImpl::init() {
    Serial.println("[PLAYBACK] Initializing playback controller");
    
    /* Get module references */
    extern AudioDecoder* create_audio_decoder();
    extern BluetoothA2DP* create_bluetooth_a2dp();
    extern SDCard* create_sd_card();
    extern UI* create_ui();
    
    decoder = create_audio_decoder();
    bt = create_bluetooth_a2dp();
    sd = create_sd_card();
    ui = create_ui();
    
    if (!decoder || !bt || !sd || !ui) {
        Serial.println("[PLAYBACK] ERROR: Could not get module references");
        return false;
    }
    
    transition_to(STATE_IDLE);
    return true;
}

PlaybackState PlaybackControllerImpl::get_state() const {
    return state;
}

void PlaybackControllerImpl::execute_command(PlaybackCommand cmd) {
    pending_cmd = cmd;
}

void PlaybackControllerImpl::update() {
    handle_command();
    update_playback();
}

uint32_t PlaybackControllerImpl::get_current_position_ms() const {
    return current_position_ms;
}

uint32_t PlaybackControllerImpl::get_total_duration_ms() const {
    return total_duration_ms;
}

/* Global singleton */
static PlaybackControllerImpl g_playback;

PlaybackController* create_playback_controller() {
    return &g_playback;
}
