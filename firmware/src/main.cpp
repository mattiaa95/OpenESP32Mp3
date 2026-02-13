/* ============================================================================
 * ESP32 BT-MP3 Player – Main Entry Point
 * FreeRTOS task initialization and event-driven orchestration
 * ========================================================================== */

#include <stdio.h>
#include "Arduino.h"
#include "sd_card.h"
#include "audio_decoder.h"
#include "bluetooth_a2dp.h"
#include "display_ssd1306.h"
#include "button_handler.h"
#include "event_queue.h"
#include "ui.h"
#include "playback_control.h"

/* Module instances */
extern SDCard* create_sd_card();
extern AudioDecoder* create_audio_decoder();
extern BluetoothA2DP* create_bluetooth_a2dp();
extern DisplaySSD1306* create_display_ssd1306();
extern ButtonHandler* create_button_handler();
extern EventQueue* create_event_queue();
extern UI* create_ui();
extern PlaybackController* create_playback_controller();

SDCard* g_sd_card = nullptr;
AudioDecoder* g_decoder = nullptr;
BluetoothA2DP* g_bt = nullptr;
DisplaySSD1306* g_display = nullptr;
ButtonHandler* g_buttons = nullptr;
EventQueue* g_event_queue = nullptr;
UI* g_ui = nullptr;
PlaybackController* g_playback = nullptr;

/* External update function */
extern void button_handler_update();

/* ============================================================================
 * Setup: Initialize all modules and prepare system
 * ========================================================================== */
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════════════════════════╗");
    Serial.println("║       ESP32 Bluetooth MP3 Player - Starting Up             ║");
    Serial.println("╚════════════════════════════════════════════════════════════╝");
    Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    /* Create event queue first (needed by other modules) */
    Serial.println("\n[INIT] Creating event queue...");
    g_event_queue = create_event_queue();
    if (!g_event_queue) {
        Serial.println("FATAL: Event queue creation failed");
        while (1) delay(1000);
    }
    
    /* Initialize display early for feedback */
    Serial.println("[INIT] Initializing display...");
    g_display = create_display_ssd1306();
    if (!g_display || !g_display->init()) {
        Serial.println("WARN: Display init failed (continuing anyway)");
    }
    
    g_display->clear();
    g_display->draw_text(10, 28, "Initializing...", 1);
    g_display->update_full();
    
    /* Initialize SD card */
    Serial.println("[INIT] Initializing SD card...");
    g_sd_card = create_sd_card();
    if (!g_sd_card || !g_sd_card->init()) {
        Serial.println("WARN: SD card init failed");
        g_display->clear();
        g_display->draw_text(5, 28, "SD Card Failed", 1);
        g_display->update_full();
        delay(2000);
    }
    
    /* List available MP3 files */
    if (g_sd_card && g_sd_card->is_mounted()) {
        const char* files[20];
        int num_files = g_sd_card->list_files(files, 20);
        Serial.printf("[INIT] Found %d MP3 files\n", num_files);
        for (int i = 0; i < num_files && i < 5; i++) {
            Serial.printf("  [%d] %s\n", i, files[i]);
        }
    }
    
    /* Initialize other modules */
    Serial.println("[INIT] Initializing audio decoder...");
    g_decoder = create_audio_decoder();
    if (!g_decoder) {
        Serial.println("WARN: Audio decoder creation failed");
    }
    
    Serial.println("[INIT] Initializing Bluetooth A2DP...");
    g_bt = create_bluetooth_a2dp();
    if (g_bt) g_bt->init();
    
    Serial.println("[INIT] Initializing UI...");
    g_ui = create_ui();
    if (!g_ui) {
        Serial.println("WARN: UI init failed");
    }
    
    Serial.println("[INIT] Initializing button handler...");
    g_buttons = create_button_handler();
    if (!g_buttons || !g_buttons->init()) {
        Serial.println("WARN: Button handler init failed");
    }
    
    Serial.println("[INIT] Initializing playback controller...");
    g_playback = create_playback_controller();
    if (!g_playback || !g_playback->init()) {
        Serial.println("WARN: Playback controller init failed");
    }
    
    /* Display ready screen */
    if (g_display) {
        g_display->clear();
        g_display->draw_text(15, 24, "Ready!", 1);
        g_display->draw_text(5, 40, "Press Play", 1);
        g_display->update_full();
    }
    
    Serial.println("\n[INIT] Initialization complete");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("════════════════════════════════════════════════════════════");
}

/* ============================================================================
 * Main Loop: Event processing and module updates
 * ========================================================================== */
void loop() {
    /* Update button state (debounce check) */
    if (g_buttons) {
        button_handler_update();
    }
    
    /* Process button events and convert to playback commands */
    if (g_buttons) {
        uint8_t button_id;
        ButtonEvent event;
        
        while (g_buttons->get_event(button_id, event)) {
            if (event != BUTTON_PRESSED) continue;  /* Ignore releases */
            
            PlaybackCommand cmd = CMD_NONE;
            switch (button_id) {
                case 0: /* PREV */
                    cmd = CMD_PLAY_PREV;
                    Serial.println("[MAIN] Button: PREV");
                    break;
                case 1: /* PLAY */
                    cmd = CMD_TOGGLE_PLAY_PAUSE;
                    Serial.println("[MAIN] Button: PLAY/PAUSE");
                    break;
                case 2: /* NEXT */
                    cmd = CMD_PLAY_NEXT;
                    Serial.println("[MAIN] Button: NEXT");
                    break;
            }
            
            if (cmd != CMD_NONE && g_playback) {
                g_playback->execute_command(cmd);
            }
        }
    }
    
    /* Update playback state machine */
    if (g_playback) {
        g_playback->update();
    }
    
    /* Update UI (every ~100 ms) */
    static uint32_t last_ui_update = 0;
    uint32_t now = millis();
    if ((now - last_ui_update) >= 100) {
        if (g_ui && g_playback) {
            g_ui->update_track_info("Test Track", "Artist Name");
            g_ui->update_progress(
                g_playback->get_current_position_ms(),
                g_playback->get_total_duration_ms()
            );
            g_ui->update_volume(80);
            g_ui->render();
        }
        last_ui_update = now;
    }
    
    /* Periodic heap check (every 5 seconds) */
    static uint32_t last_heap_check = 0;
    if ((now - last_heap_check) >= 5000) {
        uint32_t free_heap = ESP.getFreeHeap();
        if (free_heap < 20000) {
            Serial.printf("[WARN] Low heap: %d bytes\n", free_heap);
        }
        last_heap_check = now;
    }
    
    /* Yield to other tasks */
    delay(10);
}
