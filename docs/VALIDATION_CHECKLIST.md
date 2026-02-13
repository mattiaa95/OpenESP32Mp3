# ESP32 MP3 Player – Phase 10 Validation Checklist

## Hardware Validation (on Breadboard)

### Power Supply
- [ ] 5V input to USB connector (or external 5V supply)
- [ ] Multimeter check: +5V on power rail (red), GND on ground rail (black)
- [ ] No short circuits between +5V and GND
- [ ] Current draw at idle: < 150 mA
- [ ] Current draw during playback: 180–220 mA

### Wiring Verification
- [ ] **SD Card SPI**
  - [ ] GPIO 18 (CLK) → SD CLK
  - [ ] GPIO 23 (MOSI) → SD MOSI
  - [ ] GPIO 19 (MISO) → SD MISO
  - [ ] GPIO 5 (CS) → SD CS
  - [ ] Check for solder bridges, no loose wires
  
- [ ] **OLED Display I2C**
  - [ ] GPIO 21 (SDA) → SSD1306 SDA
  - [ ] GPIO 22 (SCL) → SSD1306 SCL
  - [ ] Pull-ups: Check module has 4.7 kΩ pull-ups (do NOT add external)
  - [ ] Address verification: Scan I2C bus (should find 0x3C)
  
- [ ] **Buttons**
  - [ ] GPIO 26 (PREV) → Button with 10k pull-up to +3.3V
  - [ ] GPIO 27 (PLAY) → Button with 10k pull-up to +3.3V
  - [ ] GPIO 14 (NEXT) → Button with 10k pull-up to +3.3V
  - [ ] Button pins go to GND when pressed (active-low)
  - [ ] Multimeter continuity test: Button pressed = 0V, released = 3.3V

### Component Presence
- [ ] ESP32 DevKit V1 on breadboard
- [ ] microSD card (formatted FAT32) with test MP3 files
- [ ] SSD1306 0.96" OLED display (128×64)
- [ ] SD card reader/writer module (SPI)
- [ ] 3 pushbuttons (momentary, active-low)
- [ ] Decoupling capacitors: 100 nF on +3.3V rails near ICs
- [ ] USB cable (micro-B) for power + serial

---

## Software Validation (Serial Monitor)

### Boot Sequence
```
Expected output on serial monitor (115200 baud):
[After 1 second delay from reset]
╔════════════════════════════════════════════════════════════╗
║       ESP32 Bluetooth MP3 Player - Starting Up             ║
╚════════════════════════════════════════════════════════════╝
Build: [DATE TIME]
Free heap: XXXX bytes

[INIT] Creating event queue...
[INIT] Initializing display...
[OLED] Initializing SSD1306 display
[OLED] Initialized successfully
[INIT] Initializing SD card...
[SD] SD card initialized successfully
[SD] Found X MP3 files
[SD] Listed X MP3 files
  [0] track001.mp3
  [1] track002.mp3
  ...
[INIT] Initializing audio decoder...
[INIT] Initializing Bluetooth A2DP...
[BT] Initializing Bluetooth A2DP source
[BT] Bluetooth initialized
[INIT] Initializing UI...
[UI] Initializing Flipper-style UI
[OLED] Initialized successfully
[UI] UI initialized
[INIT] Initializing button handler...
[BTN] Initializing button handler
[BTN] Configured GPIO pins: 26 (prev), 27 (play), 14 (next)
[BTN] Button handler ready
[INIT] Initializing playback controller...
[PLAYBACK] Initializing playback controller

[INIT] Initialization complete
Free heap: YYYY bytes (should be ~150 KB+)
════════════════════════════════════════════════════════════
```

### Button Presses (Serial Output)
- [ ] **Press PREV button → Serial shows:**
  ```
  [BTN] PREV PRESS
  [MAIN] Button: PREV
  [PLAYBACK] Command: 2 (state=0)
  ```
  
- [ ] **Press PLAY button → Serial shows:**
  ```
  [BTN] PLAY PRESS
  [MAIN] Button: PLAY/PAUSE
  [PLAYBACK] Command: 3 (state=0)
  [PLAYBACK] State: IDLE → LOADING
  [PLAYBACK] State: LOADING → PLAYING
  [PLAYBACK] Loaded file index 0
  ```
  
- [ ] **Press NEXT button → Serial shows:**
  ```
  [BTN] NEXT PRESS
  [MAIN] Button: NEXT
  [PLAYBACK] Command: 1 (state=2)
  [PLAYBACK] State: PLAYING → LOADING
  ```

---

## Display Validation (OLED)

### Boot Screen
- [ ] After ~1 second: "Initializing..." text appears (upper left)
- [ ] After SD check: "Ready!" + "Press Play" text
- [ ] No flicker, clean pixel rendering
- [ ] Display brightness visible (not too dim, not washed out)

### Playback Screen
- [ ] **When PLAY pressed:**
  - [ ] Title shown at top (e.g., "track001.mp3")
  - [ ] Artist shown (e.g., "Unknown" or from metadata)
  - [ ] Progress bar drawn (128 px wide at y=40)
  - [ ] Volume indicator: 10 bars on right (80% = 8 bars)
  - [ ] Play symbol (▶) in top-right corner
  
- [ ] **When PAUSED:**
  - [ ] Pause symbol (‖) shown instead of play
  - [ ] Progress bar frozen at current position
  
- [ ] **Button hints at bottom:**
  - [ ] "<<" on left (previous)
  - [ ] ">" in center (play/pause)
  - [ ] ">>" on right (next)

### Visual Quality
- [ ] No flicker during UI updates
- [ ] No ghosting or residual pixels
- [ ] Text is legible (5x7 pixel font)
- [ ] Progress bar smooth (not glitchy)
- [ ] Dirty region updates work (only changed parts redrawn)

---

## Functional Validation

### SD Card Operations
```bash
# Verify on computer before inserting:
microSD card should contain:
/
├── track001.mp3  (test file, ~1 MB)
├── track002.mp3
└── track003.mp3
(Files must be valid MP3 files with proper headers)
```

- [ ] Serial shows: "Found 3 MP3 files"
- [ ] File sizes logged: "[SD] track001.mp3 (XXXX bytes)"
- [ ] Files list updates when SD card reinserted
- [ ] Handles missing SD card gracefully (shows error on OLED)

### Audio Decoding (Phase 4 Validation)
- [ ] MP3 file opened: Serial shows frame sync detection
- [ ] Frame header parsed: Sample rate, channels, bitrate logged
- [ ] No crashes on corrupted/invalid MP3 files
- [ ] Error message displayed: "Invalid MP3" (if file corrupt)

### Bluetooth A2DP (Phase 5 Validation)
- [ ] Bluetooth A2DP mode enabled (serial shows "A2DP source ready")
- [ ] External Bluetooth speaker appears in pairing list
- [ ] Pairing PIN: 1234 (or negotiated)
- [ ] Connection status: "Bluetooth connected" appears on OLED
- [ ] Audio streams (if Phase 4 decoder ready)

### Button Responsiveness
- [ ] Button press latency < 100 ms (visual: play starts immediately)
- [ ] Debounce: Rapid press/release doesn't register multiple events
- [ ] All 3 buttons respond to input
- [ ] Held buttons don't cause rapid state changes (debounce working)

---

## Stress Testing

### Duration Test
- [ ] Play single track for 10 minutes continuously
- [ ] No crashes, no memory leaks
- [ ] Heap size stable (< 2 KB variation)
- [ ] UI updates smoothly throughout

### Long Playlist Test
- [ ] Create 10+ MP3 files on SD card
- [ ] Play through entire playlist (auto-advance on track end)
- [ ] Each track plays without dropout
- [ ] Verify all 10 tracks played before stopping

### Rapid Button Presses
- [ ] Press NEXT/PREV rapidly (10+ times) without crash
- [ ] No lockup or unresponsive state
- [ ] Serial output remains continuous

### Edge Cases
- [ ] Remove SD card during playback → Graceful pause + error message
- [ ] Insert SD card during idle → System detects and re-mounts
- [ ] Disconnect Bluetooth speaker → Serial shows "BT disconnected"
- [ ] Large MP3 file (> 100 MB) → Handles without crashing

---

## Performance Metrics

### CPU Usage
- [ ] Idle (no playback): < 5% CPU (240 MHz ESP32)
- [ ] Decoding MP3: < 15% CPU
- [ ] Bluetooth A2DP encoding: < 10% CPU
- [ ] UI rendering: < 3% CPU
- [ ] Total (playback): < 25% CPU (headroom for ISRs)

### Memory
- [ ] Boot: 34 KB used (target: < 50 KB)
- [ ] After 10 min playback: No heap fragmentation
- [ ] Ring buffer watermark: Stays between 25–75% full
- [ ] No malloc after startup (all pre-allocated)

### Audio Quality (Phase 5+)
- [ ] No dropouts (should hear continuous audio)
- [ ] Correct sample rate (verify tone from 44.1 kHz file)
- [ ] Correct channels (stereo files play stereo)
- [ ] No distortion or clipping
- [ ] Volume control works (0–100%)

### Power Consumption
- [ ] Idle (no playback): 80–100 mA
- [ ] Display on, playback stopped: 100–120 mA
- [ ] Playback active (decoder + BT): 150–180 mA
- [ ] Peak (all subsystems active): < 220 mA

---

## Troubleshooting Quick Reference

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| OLED not initializing | I2C wiring | Check GPIO 21/22, scan I2C (address 0x3C) |
| SD card not detected | SPI wiring | Check GPIO 18/23/19/5, verify CS pin |
| Buttons not responsive | GPIO wiring | Multimeter check voltage (0V pressed, 3.3V released) |
| Crashes on startup | Heap exhaustion | Check free heap at boot (should be > 150 KB) |
| Garbled text on OLED | Display contrast | Adjust set_contrast() in display_ssd1306.cpp |
| Audio dropout | Ring buffer underrun | Increase buffer size in config.h |
| Bluetooth won't pair | Address conflict | Ensure 0x3C in SSD1306, 0x48+ for RTC if used |
| Rapid button re-triggering | Debounce too short | Increase BTN_DEBOUNCE_MS in config.h to 30 ms |

---

## Final Sign-Off

- [ ] All hardware tests passed
- [ ] All software tests passed
- [ ] All functional tests passed
- [ ] Stress tests completed (no crashes/memory leaks)
- [ ] Performance metrics acceptable
- [ ] Audio quality verified
- [ ] Device ready for production use

**Date Tested:** _______________
**Tester Name:** _______________
**Signature:** _______________

---

*Validation Checklist — Phase 10 — ESP32 MP3 Player v1.0*
