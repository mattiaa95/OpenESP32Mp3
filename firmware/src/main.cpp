/* ============================================================================
 * ESP32 BT-MP3 Player – Main Entry Point
 * FreeRTOS task initialization and orchestration
 * ========================================================================== */

#include <stdio.h>
#include "Arduino.h"

static const char* TAG = "main";

void setup() {
    Serial.begin(115200);
    delay(1000);  // Wait for serial to stabilize
    
    Serial.println("\n\n=== ESP32 Bluetooth MP3 Player ===");
    Serial.println("Firmware starting...");
    
    /* Initialize all modules here (Phase 1: stubs) */
    // TODO: Initialize event queue
    // TODO: Initialize SD card
    // TODO: Initialize audio decoder
    // TODO: Initialize Bluetooth A2DP
    // TODO: Initialize display
    // TODO: Initialize button handler
    
    /* Create FreeRTOS tasks */
    // TODO: xTaskCreatePinnedToCore(audio_decode_task, ...);
    // TODO: xTaskCreatePinnedToCore(bluetooth_feed_task, ...);
    // TODO: xTaskCreatePinnedToCore(display_refresh_task, ...);
    // TODO: xTaskCreatePinnedToCore(button_debounce_task, ...);
    // TODO: xTaskCreatePinnedToCore(playback_control_task, ...);
    // TODO: xTaskCreatePinnedToCore(sd_card_task, ...);
    
    Serial.println("All tasks started");
}

void loop() {
    delay(1000);
    Serial.println("Main loop heartbeat");
}

