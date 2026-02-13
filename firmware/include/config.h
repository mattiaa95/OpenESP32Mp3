#ifndef CONFIG_H
#define CONFIG_H

/* ============================================================================
 * ESP32-BT-MP3-Player Configuration Header
 * Hardware pin definitions, buffer sizes, I2C/SPI addresses, timeouts
 * ========================================================================== */

/* ============================================================================
 * ESP32 SPI Configuration (SD Card via SPI2)
 * ========================================================================== */
#define SPI_MOSI_PIN    23   // GPIO 23 (D23)
#define SPI_MISO_PIN    19   // GPIO 19 (D19)
#define SPI_CLK_PIN     18   // GPIO 18 (D18)
#define SPI_CS_PIN      5    // GPIO 5  (D5) – SD card chip select

#define SPI_CLOCK_FREQ  20   // MHz (conservative; can push to 40 MHz)
#define SPI_HOST        VSPI_HOST  // ESP32 SPI2 peripheral

/* ============================================================================
 * ESP32 I2C Configuration (SSD1306 OLED via I2C0)
 * ========================================================================== */
#define I2C_SDA_PIN     21   // GPIO 21 (D21)
#define I2C_SCL_PIN     22   // GPIO 22 (D22)
#define I2C_FREQ        100  // kHz (standard mode; SSD1306 supports 100–400 kHz)
#define I2C_PORT        I2C_NUM_0

/* SSD1306 I2C Address */
#define SSD1306_I2C_ADDR 0x3C

/* ============================================================================
 * GPIO Button Configuration (Pull-up, active-low)
 * ========================================================================== */
#define BTN_PREV_PIN    26   // GPIO 26 – Previous track button
#define BTN_PLAY_PIN    27   // GPIO 27 – Play/Pause button
#define BTN_NEXT_PIN    14   // GPIO 14 – Next track button

#define BTN_DEBOUNCE_MS 20   // Debounce window (milliseconds)

/* ============================================================================
 * Audio Buffer Configuration
 * ========================================================================== */
#define AUDIO_RING_BUFFER_SIZE  (64 * 1024)  // 64 KB ring buffer (~910 ms @ 44.1 kHz stereo)
#define AUDIO_SAMPLE_RATE       44100        // Hz
#define AUDIO_CHANNELS          2            // Stereo
#define AUDIO_BITS_PER_SAMPLE   16           // Bits

/* Derived: bytes per second */
#define AUDIO_BYTES_PER_SEC (AUDIO_SAMPLE_RATE * AUDIO_CHANNELS * AUDIO_BITS_PER_SAMPLE / 8)

/* MP3 Decoder Frame Size (typical) */
#define MP3_MAX_FRAME_SIZE      2048    // Bytes (1.4–1.8 KB typical, max ~2 KB)
#define MP3_INPUT_BUFFER_SIZE   (2 * MP3_MAX_FRAME_SIZE)

/* ============================================================================
 * OLED Display Configuration
 * ========================================================================== */
#define DISPLAY_WIDTH   128  // Pixels
#define DISPLAY_HEIGHT  64   // Pixels
#define DISPLAY_PAGES   (DISPLAY_HEIGHT / 8)  // 8 pages (each = 8 vertical bits)
#define DISPLAY_REFRESH_HZ 10  // UI refresh rate

/* Frame buffer size (128 × 64 / 8 = 1024 bytes) */
#define FRAMEBUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_PAGES)

/* ============================================================================
 * Event Queue Configuration
 * ========================================================================== */
#define EVENT_QUEUE_SIZE 20  // Maximum pending events

/* ============================================================================
 * FreeRTOS Task Configuration
 * ========================================================================== */

/* Task stack sizes (in words, not bytes; word = 4 bytes on ESP32) */
#define TASK_STACK_WORDS_MAIN           2048  // Main task
#define TASK_STACK_WORDS_AUDIO          2048  // Audio decode task
#define TASK_STACK_WORDS_BT             2048  // Bluetooth A2DP task
#define TASK_STACK_WORDS_DISPLAY        2048  // Display refresh task
#define TASK_STACK_WORDS_BUTTON         1024  // Button debounce task
#define TASK_STACK_WORDS_PLAYBACK       2048  // Playback control task
#define TASK_STACK_WORDS_SD             2048  // SD card task

/* Task priorities (FreeRTOS: higher number = higher priority) */
#define TASK_PRIORITY_AUDIO_DECODE      24    // High: must not block
#define TASK_PRIORITY_BT_A2DP           23    // High: feeds Bluetooth
#define TASK_PRIORITY_DISPLAY           22    // Medium: UI refresh
#define TASK_PRIORITY_BUTTON            20    // Medium: debounce
#define TASK_PRIORITY_PLAYBACK_CONTROL  15    // Normal: orchestration
#define TASK_PRIORITY_SD                10    // Low: file I/O

/* ============================================================================
 * Timeouts (milliseconds)
 * ========================================================================== */
#define SD_INIT_TIMEOUT_MS    5000  // SD card initialization timeout
#define SD_READ_TIMEOUT_MS    1000  // Single block read timeout
#define BT_CONNECT_TIMEOUT_MS 10000 // Bluetooth pairing timeout
#define I2C_TIMEOUT_MS        1000  // I2C transaction timeout

/* ============================================================================
 * Logging Configuration
 * ========================================================================== */
#define LOG_LEVEL_MAIN        "INFO"
#define LOG_LEVEL_AUDIO       "DEBUG"
#define LOG_LEVEL_BT          "INFO"
#define LOG_LEVEL_DISPLAY     "DEBUG"
#define LOG_LEVEL_BUTTON      "DEBUG"

/* ============================================================================
 * Feature Flags (for conditional compilation)
 * ========================================================================== */
#define FEATURE_BLUETOOTH_A2DP   1  // Enable Bluetooth A2DP
#define FEATURE_OLED_DISPLAY     1  // Enable OLED display
#define FEATURE_BUTTON_CONTROLS  1  // Enable button input
#define FEATURE_SD_CARD          1  // Enable SD card

#if AUDIO_RING_BUFFER_SIZE < 8192
    #error "Audio ring buffer must be >= 8 KB"
#endif

#if DISPLAY_WIDTH != 128 || DISPLAY_HEIGHT != 64
    #error "SSD1306 display must be 128x64 pixels"
#endif

#endif  // CONFIG_H
