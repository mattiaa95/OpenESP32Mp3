/* ============================================================================
 * ESP32 BT-MP3 Player – Main Entry Point
 * FreeRTOS task initialization and orchestration
 * ========================================================================== */

#include <stdio.h>
#include "Arduino.h"
#include "sd_card.h"
#include "audio_decoder.h"

static const char* TAG = "main";

// Create global instances
extern SDCard* create_sd_card();
extern AudioDecoder* create_audio_decoder();

SDCard* g_sd_card = nullptr;
AudioDecoder* g_decoder = nullptr;

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
            delay(1000);
        }
    }
    
    /* List MP3 files */
    const char* files[20];
    int num_files = g_sd_card->list_files(files, 20);
    Serial.printf("Found %d MP3 files on SD card\n", num_files);
    for (int i = 0; i < num_files; i++) {
        Serial.printf("  [%d] %s\n", i, files[i]);
    }
    
    /* Initialize MP3 Decoder */
    g_decoder = create_audio_decoder();
    if (!g_decoder) {
        Serial.println("FATAL: Could not create decoder");
        while (1) delay(1000);
    }
    
    /* Test: Open first MP3 file if available */
    if (num_files > 0 && g_sd_card->open_file(files[0])) {
        Serial.printf("[MAIN] Opened MP3: %s\n", files[0]);
        
        /* Read first 64 bytes */
        uint8_t header[64];
        int bytes = g_sd_card->read_data(header, 64);
        Serial.printf("[MAIN] Read %d bytes from file\n", bytes);
        
        if (bytes > 0) {
            Serial.print("[MAIN] Header hex: ");
            for (int i = 0; i < (bytes < 16 ? bytes : 16); i++) {
                Serial.printf("%02X ", header[i]);
            }
            Serial.println();
        }
        
        g_sd_card->close_file();
    }
    
    /* TODO: Initialize other modules */
    // - Event queue
    // - Bluetooth A2DP
    // - Display
    // - Button handler
    
    /* TODO: Create FreeRTOS tasks */
    // xTaskCreatePinnedToCore(...);
    
    Serial.println("Setup complete");
}

void loop() {
    delay(5000);
    Serial.println("Main loop heartbeat");
}

