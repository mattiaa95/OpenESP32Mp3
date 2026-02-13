#include "ui.h"
#include "display_ssd1306.h"
#include "config.h"
#include <Arduino.h>
#include <cstring>

/* ============================================================================
 * Flipper-Style UI Implementation
 * Minimal monochrome layout: title, artist, progress, volume, state indicator
 * ========================================================================== */

class UIImpl : public UI {
private:
    DisplaySSD1306* display;
    
    char current_title[64] = "No Track";
    char current_artist[64] = "";
    PlayerState state = UI_STATE_IDLE;
    uint32_t progress_ms = 0;
    uint32_t duration_ms = 0;
    uint8_t volume = 80;
    
    char error_msg[64] = "";
    uint32_t error_time_ms = 0;
    static const uint32_t ERROR_DISPLAY_TIME = 3000;  /* 3 seconds */
    
    char info_msg[64] = "";
    uint32_t info_time_ms = 0;
    static const uint32_t INFO_DISPLAY_TIME = 2000;  /* 2 seconds */
    
    void draw_header();
    void draw_body();
    void draw_footer();
    void draw_progress_bar();
    void draw_state_indicator();
    
public:
    bool init() override;
    void update_track_info(const char* title, const char* artist) override;
    void update_playback_state(PlayerState s) override;
    void update_progress(uint32_t curr_ms, uint32_t dur_ms) override;
    void update_volume(uint8_t vol) override;
    void render() override;
    void show_error(const char* message) override;
    void show_info(const char* message) override;
};

bool UIImpl::init() {
    Serial.println("[UI] Initializing Flipper-style UI");
    
    /* Get display instance */
    extern DisplaySSD1306* create_display_ssd1306();
    display = create_display_ssd1306();
    
    if (!display || !display->init()) {
        Serial.println("[UI] FATAL: Could not initialize display");
        return false;
    }
    
    /* Clear and show welcome screen */
    display->clear();
    display->draw_text(10, 28, "ESP32 MP3", 1);
    display->update_full();
    
    Serial.println("[UI] UI initialized");
    return true;
}

void UIImpl::update_track_info(const char* title, const char* artist) {
    if (title) strncpy(current_title, title, sizeof(current_title) - 1);
    if (artist) strncpy(current_artist, artist, sizeof(current_artist) - 1);
}

void UIImpl::update_playback_state(PlayerState s) {
    state = s;
}

void UIImpl::update_progress(uint32_t curr_ms, uint32_t dur_ms) {
    progress_ms = curr_ms;
    duration_ms = dur_ms;
}

void UIImpl::update_volume(uint8_t vol) {
    volume = vol;
}

void UIImpl::draw_state_indicator() {
    /* Draw state icon in top-right */
    uint8_t x = 120, y = 2;
    uint8_t w = 6, h = 6;
    
    if (state == UI_STATE_PLAYING) {
        /* Play symbol: right-pointing triangle */
        display->draw_pixel(x + 1, y + 2, 1);
        display->draw_pixel(x + 2, y + 1, 1);
        display->draw_pixel(x + 2, y + 3, 1);
        display->draw_pixel(x + 3, y + 2, 1);
    } else if (state == UI_STATE_PAUSED) {
        /* Pause symbol: two vertical bars */
        display->draw_vline(x + 1, y + 1, 4, 1);
        display->draw_vline(x + 3, y + 1, 4, 1);
    } else if (state == UI_STATE_LOADING) {
        /* Loading symbol: circle outline */
        display->draw_rect(x, y, 6, 6, 1);
    }
}

void UIImpl::draw_progress_bar() {
    /* Progress bar at y=40, 128 pixels wide */
    uint8_t y = 40;
    uint8_t bar_height = 2;
    
    /* Background (outline) */
    display->draw_rect(0, y, DISPLAY_WIDTH, bar_height + 2, 1);
    
    /* Progress fill */
    if (duration_ms > 0) {
        uint32_t filled_width = (progress_ms * DISPLAY_WIDTH) / duration_ms;
        if (filled_width > DISPLAY_WIDTH) filled_width = DISPLAY_WIDTH;
        
        if (filled_width > 0) {
            display->draw_filled_rect(1, y + 1, filled_width - 1, bar_height, 1);
        }
    }
}

void UIImpl::draw_header() {
    /* Title (top 8 pixels) */
    display->draw_text(2, 0, current_title, 1);
    
    /* Horizontal divider at y=10 */
    display->draw_hline(0, 10, DISPLAY_WIDTH, 1);
}

void UIImpl::draw_body() {
    /* Artist (middle section) */
    display->draw_text(2, 20, current_artist, 1);
    
    /* Progress bar */
    draw_progress_bar();
    
    /* Volume on right side (y=45) */
    uint8_t vol_x = 110;
    display->draw_text(vol_x, 45, "V", 1);
    
    /* Volume level (0–100) → 1–10 bars */
    uint8_t bars = (volume / 10);
    for (uint8_t i = 0; i < bars && i < 10; i++) {
        display->draw_vline(vol_x + 6 + i, 50 - (i / 2), 1 + (i / 2), 1);
    }
}

void UIImpl::draw_footer() {
    /* Button hints at bottom */
    display->draw_hline(0, 58, DISPLAY_WIDTH, 1);
    
    /* Left: <<  Middle: Play  Right: >> */
    display->draw_text(2, 60, "<<", 1);
    display->draw_text(60, 60, ">", 1);
    display->draw_text(118, 60, ">>", 1);
    
    /* State indicator in top-right */
    draw_state_indicator();
}

void UIImpl::render() {
    /* Check if error or info messages are active */
    uint32_t now = millis();
    
    if (error_msg[0] && (now - error_time_ms) < ERROR_DISPLAY_TIME) {
        /* Show error overlay */
        display->clear();
        display->draw_filled_rect(0, 20, DISPLAY_WIDTH, 24, 0);  /* Invert background */
        display->draw_rect(0, 20, DISPLAY_WIDTH, 24, 1);
        display->draw_text(5, 28, error_msg, 0);  /* White text on black */
        display->update_full();
        return;
    } else if (error_msg[0]) {
        error_msg[0] = '\0';
    }
    
    if (info_msg[0] && (now - info_time_ms) < INFO_DISPLAY_TIME) {
        /* Show info toast */
        display->draw_filled_rect(10, 2, DISPLAY_WIDTH - 20, 12, 0);
        display->draw_rect(10, 2, DISPLAY_WIDTH - 20, 12, 1);
        display->draw_text(15, 4, info_msg, 0);
    } else if (info_msg[0]) {
        info_msg[0] = '\0';
    }
    
    /* Normal rendering */
    display->clear();
    draw_header();
    draw_body();
    draw_footer();
    display->update();  /* Partial update */
}

void UIImpl::show_error(const char* message) {
    if (message) {
        strncpy(error_msg, message, sizeof(error_msg) - 1);
        error_msg[sizeof(error_msg) - 1] = '\0';
        error_time_ms = millis();
    }
}

void UIImpl::show_info(const char* message) {
    if (message) {
        strncpy(info_msg, message, sizeof(info_msg) - 1);
        info_msg[sizeof(info_msg) - 1] = '\0';
        info_time_ms = millis();
    }
}

/* Global singleton */
static UIImpl g_ui;

UI* create_ui() {
    return &g_ui;
}
