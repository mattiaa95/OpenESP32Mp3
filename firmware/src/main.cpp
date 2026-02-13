/* ============================================================================
 * ESP32 BT-MP3 Player – Main Entry Point
 * FreeRTOS task initialization and orchestration
 * ========================================================================== */

#include <stdio.h>
#include "Arduino.h"
#include "sd_card.h"

static const char* TAG = "main";

// Create global SD card instance
extern SDCard* create_sd_card();
SDCard* g_sd_card = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);  // Wait for serial to stabilize
    
    Serial.println("\n\n=== ESP32 Bluetooth MP3 Player ===");
    Serial.println("Firmware starting...");
    Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
    
    /* Initialize SD Card */
    g_sd_card = create_sd_card();
    if (!g_sd_card->init()) {
        Serial.println("FATAL: SD card initialization failed");
        while (1) {
            delay(1000);  // Halt
        }
    }
    
    /* List MP3 files */
    const char* files[20];
    int num_files = g_sd_card->list_files(files, 20);
    Serial.printf("Found %d MP3 files on SD card\n", num_files);
    for (int i = 0; i < num_files; i++) {
        Serial.printf("  [%d] %s\n", i, files[i]);
    }
    
    /* TODO: Initialize other modules */
    // - Event queue
    // - Audio decoder
    // - Bluetooth A2DP
    // - Display
    // - Button handler
    
    /* TODO: Create FreeRTOS tasks */
    // xTaskCreatePinnedToCore(...);
    
    Serial.println("Setup complete");
}

void loop() {
    delay(1000);
    Serial.println("Main loop heartbeat");
}

