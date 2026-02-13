# PHASE 0 – SKILL BOOTSTRAPPING
## Engineering Knowledge Requirements

This document captures essential skill domains required to architect, implement, and validate the ESP32-based Bluetooth MP3 Player. Each domain includes precise definitions, micro knowledge briefs, best practices, pitfalls, and validation criteria.

---

## 1. ESP32 ARCHITECTURE & CAPABILITIES

### Definition
Understanding ESP32 hardware capabilities, memory layout, boot sequence, and peripheral integration (SPI, I2C, GPIO, UART).

### Micro Knowledge Brief
**ESP32 Variants for This Project:**
- **ESP32 (Standard):** Dual-core Xtensa LX6 @ 240 MHz, 520 KB SRAM, 4 MB flash. Adequate for this project. **SELECTED** for cost/availability.
- **ESP32-S3:** Higher performance, USB native, same RAM/flash. Overkill; unnecessary cost.
- **ESP32-C3:** Single-core, RISC-V, lower power. Insufficient for concurrent MP3 decoding + Bluetooth.
- **Memory Layout:** IRAM0 (32 KB), IRAM1 (32 KB), DRAM (520 KB total). Audio buffers + Bluetooth stack consume 150–200 KB. Heap fragmentation is critical.

**Key Peripheral Architecture:**
- **SPI:** 4 instances (SPI1 reserved for flash). Use SPI2/SPI3 for SD card @ 20 MHz (conservative). DMA capable.
- **I2C:** 2 instances. SSD1306 @ standard mode (100 kHz). No clock stretching issues expected.
- **GPIO:** 40 pins. Debounced input pins required for button handling.
- **UART:** 3 instances (UART0 reserved for debug). Use for logging only.

**Boot & Power:**
- Bootloader runs from flash (2 stages).
- FreeRTOS kernel manages task scheduling.
- Bluetooth requires persistent low-level firmware (IDF components).

### Best Practices
1. **Memory Management:**
   - Pre-allocate fixed ring buffers for audio (avoid dynamic allocation in real-time loops).
   - Use SPIRAM (external RAM) if available; fallback to heap with careful sizing.
   - Profile heap fragmentation with `heap_caps_print_heap_stats()`.

2. **Clock & Frequency:**
   - CPU @ 240 MHz for normal operation.
   - GPIO rising/falling times: ≤ 200 ns (debounce: 20 ms).
   - SPI @ 20 MHz for SD (conservative; can clock up to 40 MHz with good layout).
   - I2C @ 100 kHz or 400 kHz (SSD1306 supports both).

3. **Interrupt Handling:**
   - Keep ISRs < 5 μs.
   - Defer work to task/queue instead of blocking ISR.
   - Use `portENTER_CRITICAL()` sparingly.

4. **Peripheral Multiplexing:**
   - SPI2 → SD card (MOSI, MISO, CLK, CS).
   - I2C0 → SSD1306 (SDA, SCL).
   - GPIO → 3 buttons (level-triggered, edge-triggered with debounce).

### Common Pitfalls
- **Over-clocking SPI:** Causes CRC errors on SD reads → audio artifacts.
- **Blocking I2C operations:** If SSD1306 write blocks Bluetooth ISR, audio dropout.
- **Unbounded heap allocation:** Causes fragmentation; prefer pre-allocated buffers.
- **Multiple tasks accessing SPI without mutex:** Race conditions on card reads.
- **Not using task priorities:** Bluetooth ISR starves UI redraw task.

### Validation Criteria
- ✅ Serial output confirms CPU @ 240 MHz.
- ✅ Heap snapshot shows stable fragmentation (< 30% wasted).
- ✅ GPIO edge detection confirmed in interrupt logs.
- ✅ SPI clock measured with oscilloscope @ 20 MHz ± 5%.
- ✅ I2C ACK/NAK sequences captured on oscilloscope.

---

## 2. BLUETOOTH A2DP SOURCE STREAMING

### Definition
Establishing ESP32 as Bluetooth A2DP Source (audio provider) to stream MP3-decoded PCM audio to external Bluetooth speakers/headphones.

### Micro Knowledge Brief
**A2DP Protocol Stack:**
- A2DP = Audio Distribution Profile (Bluetooth SIG standard).
- Source (our ESP32) → Sink (speaker/headphones).
- Transport: L2CAP (Logical Link Control & Adaptation Protocol).
- Codec negotiation: SBC (mandatory), optionally MPEG-1/2 (MP3), AAC, aptX.

**ESP-IDF Bluetooth Components:**
- **BTstack:** Bluetooth classic stack (not BLE).
- **`esp_avrc_ct_init()`:** Audio/Video Remote Control (Sink side; we need Source).
- **`esp_a2dp_source_init()`:** A2DP Source initialization.
- **`esp_a2dp_source_connect()`:** Initiate pairing.
- **Audio Data Callback:** `int32_t esp_a2dp_source_data_source()` feeds PCM samples (44.1 kHz, 16-bit stereo).

**Audio Pipeline for A2DP Source:**
```
[SD Card] → [SPI] → [MP3 Decoder] → [Ring Buffer] → [A2DP Encoder] → [Bluetooth HW] → [Speaker]
```

**SBC Encoding (Mandatory Codec):**
- Input: 44.1 kHz, 16-bit stereo PCM (176 bytes/20 ms frame).
- Output: Bitstream @ 320 kbps typical.
- Latency: ≈ 50 ms (acceptable for audio playback).

**Pairing & Bonding:**
- PIN: 0000 (default; hardcoded for simplicity).
- Device discovery: Requires custom gap_advertise callback.
- Persistent: Store bonded device address in NVS (Non-Volatile Storage).

### Best Practices
1. **Ring Buffer Architecture:**
   - Minimum 2 × 20 ms buffers (88 samples × 2 channels × 2 bytes) = 704 bytes × 2 = 1.4 KB.
   - Realistic: 100–200 ms ring buffer (500 KB for 16-bit stereo @ 44.1 kHz).
   - Use producer/consumer model with atomic `head`/`tail` pointers.

2. **Bluetooth Task Isolation:**
   - Dedicate a high-priority task (Priority 24, just below Bluetooth ISR) to feed audio data.
   - Never block this task; use queues for coordination.

3. **Codec Selection:**
   - Use SBC (always available, CPU cost ≈ 5–10% @ 240 MHz).
   - MP3 codec negotiation may fail on older speakers; SBC is universal fallback.

4. **Latency Management:**
   - Target ≤ 200 ms end-to-end (SD→BT→Speaker).
   - Pre-buffer 100 ms before connecting (avoid underrun on first packets).

5. **Connection Lifecycle:**
   - Disconnect gracefully on power-off (send AVDTP stop, wait for ACK).
   - Detect speaker disconnect (monitor AVDTP state change callbacks).
   - Auto-reconnect to last bonded device on boot.

### Common Pitfalls
- **Insufficient Audio Buffering:** Underruns → silence gaps, speaker auto-disconnects.
- **Blocking Calls in Audio Callback:** Stops A2DP encoder; instant audio dropout.
- **SBC Bitstream Corruption:** Audio crackle/distortion.
- **Pairing Failure on Power Cycle:** Not bonding credentials properly in NVS.
- **No Recovery After Bluetooth Disconnect:** Task hangs waiting for reconnection.

### Validation Criteria
- ✅ Bluetooth device scanned and visible on external device.
- ✅ Pairing PIN prompt appears on speaker.
- ✅ Connection established; Bluetooth LED indicator on ESP32.
- ✅ Audio data callback invoked every 20 ms.
- ✅ PCM samples flow to SBC encoder without buffer underrun.
- ✅ Audio heard on speaker (no crackling or dropouts).
- ✅ Graceful reconnect after speaker power-off/on.

---

## 3. MP3 DECODING ON MICROCONTROLLERS

### Definition
Real-time decompression of MP3 bitstream to PCM audio (44.1 kHz, 16-bit stereo) on resource-constrained ESP32.

### Micro Knowledge Brief
**MP3 Format Essentials:**
- **Header:** Sync word (0xFFF) + MPEG version + bitrate + sample rate.
- **Frame Size:** ≈ 1.4 KB @ 128 kbps, 44.1 kHz (varies with bitrate).
- **Sample Rate:** Typically 44.1 kHz, 48 kHz.
- **Channels:** Mono (1) or Stereo (2).
- **Decoding:** Extract frames → De-huffman → Inverse quantization → IMDCT → PCM output.

**Decoder Options for ESP32:**
1. **libhelix (Perforce Helix):**
   - ARM NEON-optimized MP3 decoder (~3% CPU @ 240 MHz, 128 kbps mono).
   - Supports MPEG-1/2 layers I, II, III.
   - ~150 KB code + ~20 KB state per instance.
   - **SELECTED** for balance of performance & compatibility.

2. **minimp3:**
   - Single-file, lightweight (~300 lines).
   - ~5–8% CPU @ 240 MHz.
   - Limited optimization.

3. **esp_codec_dev (IDF built-in):**
   - Uses external codec IC (not applicable here; would require hardware).

**Audio Frame Processing Pipeline:**
```
[Read MP3 frame] → [Find sync header] → [Parse frame header] → [Decode samples] → [Scale to 16-bit] → [Ring buffer] → [A2DP or OLED meter]
```

**Memory Requirements:**
- MP3 decoder state: ≈ 20 KB (fixed per decoder instance).
- Input buffer: ≈ 2 KB (one frame).
- Output buffer: ≈ 4 KB (stereo PCM frame, up to 1152 samples).
- Total: ~26 KB per stream (highly acceptable on 520 KB DRAM).

### Best Practices
1. **Frame Synchronization:**
   - Read SD card in chunks (4–8 KB).
   - Scan for MP3 frame sync (0xFFF) using byte-by-byte search.
   - Cache next frame while decoding current frame (pipelining).

2. **Error Recovery:**
   - CRC/bitstream errors are recoverable; skip corrupted frame, seek next sync.
   - Don't propagate errors to user; log and continue.
   - Silence on checksum mismatch rather than crashing.

3. **Timing & Buffering:**
   - Decode in 20 ms chunks (88 samples @ 44.1 kHz, stereo).
   - Don't decode faster than real-time (causes audio glitches if A2DP underruns).
   - Pre-buffer 100–200 ms before playback.

4. **Memory Efficiency:**
   - Use single decoder instance; no parallel decoding.
   - Reuse input/output buffers (circular, pre-allocated).
   - Profile with `heap_caps_get_largest_free_block()`.

### Common Pitfalls
- **Frame Sync Failure:** Corrupted MP3 files cause decoder to lose sync; output garbage.
- **Underflow Handling:** If SD reads stall, decoder output stalls → A2DP underrun.
- **Integer Overflow:** PCM scaling can overflow; use proper headroom.
- **Not Handling VBR (Variable Bitrate):** VBR files have non-uniform frame sizes; bitrate guessing fails.
- **CPU Saturation:** Decode task starves Bluetooth ISR → audio dropout.

### Validation Criteria
- ✅ MP3 file read from SD card.
- ✅ Frame sync detected (0xFFF header found).
- ✅ Stereo PCM output verified (left/right channels distinct).
- ✅ Audio duration calculated correctly (frames × sample rate).
- ✅ CPU usage < 10% @ 240 MHz (headroom for Bluetooth, UI).
- ✅ No buffer underruns during playback.
- ✅ VBR files decoded correctly (song progresses at real-time speed).

---

## 4. SPI SD CARD COMMUNICATION

### Definition
Interfacing with SD/microSD card via SPI protocol (Master: ESP32, Slave: SD card) for file I/O.

### Micro Knowledge Brief
**SD Card Protocol Over SPI:**
- **Speed:** Up to 50 MHz in native mode; SPI mode (our use case) @ 20 MHz typical.
- **Initialization:** Send 80 clock cycles, then CMD0 (reset), CMD8 (voltage check), ACMD41 (operating condition), CMD58 (read OCR), CMD16 (set block size = 512 bytes).
- **Data Transfer:** CMD17 (read single block, 512 bytes), CRC16 check.
- **Power Management:** 3.3 V required; logic level shift optional if using 5 V tolerant GPIO.

**SD Card Command Packet:**
```
[Start Bit] [Tx Bit] [Cmd Index (6)] [Argument (32)] [CRC7 (7)] [Stop Bit]
Example: CMD17 (read block) = 0x51, 0x00, 0x00, 0x00, 0x00, 0x00 (4-byte block address, CRC, stop)
```

**SD Responses:**
- **R1:** 1 byte (status flags).
- **R7:** 1 byte status + 4 bytes (voltage + echo).
- **R3/R7:** Contains OCR (Operating Condition Register).
- **Data Block:** 512 bytes + 2-byte CRC16 (optional, often disabled).

**ESP32 SPI Configuration:**
- **SPI2 peripheral:** MOSI (GPIO 23), MISO (GPIO 19), CLK (GPIO 18), CS (GPIO 5).
- **DMA:** Enable for large transfers (reduce CPU overhead).
- **Clock Divider:** APB clock (80 MHz) / 4 = 20 MHz (conservative; can push to 40 MHz with good layout).

### Best Practices
1. **Initialization Sequence:**
   - Power-on delay: ≥ 1 ms.
   - Send 80+ clock cycles (10 bytes 0xFF) before CMD0.
   - Retry CMD41 up to 100× (SD card initialization can be slow).
   - Verify CRC & CMD echo before proceeding.

2. **Error Handling:**
   - Check R1 response flags after every command (idle, illegal command, CRC error, etc.).
   - Implement timeout (100 ms) on blocking transfers.
   - Gracefully degrade: log error, skip corrupted block, resume.

3. **Buffering & DMA:**
   - Pre-allocate 4 KB DMA-aligned buffer for SD reads.
   - Use DMA for transfers > 512 bytes (non-blocking).
   - Synchronize DMA completion with event flag before accessing buffer.

4. **File System:**
   - Use FAT32 driver (esp_vfs_fat.h from IDF) rather than raw block I/O.
   - Alternatively: Lightweight FATFS library (ChaN's FatFs).
   - Directory enumeration for track listing.

5. **Power Consumption:**
   - SD card idle: ≤ 100 µA.
   - Read @ 20 MHz: ≈ 100–200 mA (short bursts).
   - Implement read-ahead caching to minimize card activity.

### Common Pitfalls
- **Slow Clock Initialization:** Using full 20 MHz from boot causes init to fail; ramp from 400 kHz to 20 MHz.
- **CRC Disabled During Init:** Standard practice; enable after init complete.
- **Insufficient CS Hold Time:** CS must deassert after last MISO bit; add delay.
- **DMA Buffer Not Aligned:** Data corruption; use `DRAM_ATTR` or heap_caps_malloc for alignment.
- **No Timeout on Block Reads:** Card unresponsive → task hangs indefinitely.

### Validation Criteria
- ✅ SD card detected (CMD0, CMD8, ACMD41 succeed).
- ✅ Block size set to 512 bytes (CMD16).
- ✅ Single-block read (CMD17) returns correct data (verify with known file).
- ✅ Multi-block read (CMD18) completes without CRC errors.
- ✅ File system enumeration works (FAT32 directory listing).
- ✅ Large file read (> 1 MB) completes at expected speed (≥ 1 MB/sec @ 20 MHz).
- ✅ Graceful error on corrupted card (no infinite loop).

---

## 5. I2C SSD1306 OLED DISPLAY HANDLING

### Definition
Controlling 0.96" monochrome OLED display (SSD1306) via I2C to render UI.

### Micro Knowledge Brief
**SSD1306 Specifications:**
- **Resolution:** 128 × 64 pixels (monochrome).
- **Interface:** I2C (0x3C default address, 100/400 kHz).
- **Command Set:** Initialization (contrast, clock, multiplex ratio), memory addressing, page/column addressing.
- **Framebuffer:** 128 × 8 pages (each page = 8 vertical bits, 1 byte per column).

**I2C Frame Structure:**
```
[Start] [Address+W] [ACK] [0x00 (cmd/data)] [Payload (1–32 bytes)] [ACK] [Stop]
```

**SSD1306 Memory Map:**
- **Pages 0–7:** Vertical segments (8 rows × 128 columns = 1024 bytes).
- **Column addressing:** 0–127 (left to right).
- **Page addressing:** Set page index, column range, write data.

**Initialization Commands:**
1. Set display OFF (0xAE).
2. Set clock divisor (0xD5, 0x80).
3. Set multiplex ratio (0xA8, 0x3F for 64 rows).
4. Set display offset (0xD3, 0x00).
5. Set start line (0x40).
6. Set COM output scan direction (0xC8 for reversed).
7. Set segment re-map (0xA1 for reversed).
8. Set contrast (0x81, 0x7F).
9. Display ON (0xAF).

### Best Practices
1. **Framebuffer Strategy:**
   - **Option A (RAM buffer):** Maintain 1024-byte in-memory framebuffer, dirty-track regions, partial redraw.
   - **Option B (Direct write):** Write primitives directly to display (slower; minimal RAM).
   - **SELECTED:** Option A with dirty-region tracking (smooth, flicker-free).

2. **Write Efficiency:**
   - Use I2C DMA if available (enable in IDF config).
   - Batch multiple bytes per transaction (16–32 bytes per I2C write).
   - Non-blocking writes (queue to task, deferred update).

3. **Content Rendering:**
   - Use monospace font for track name (max 16 chars per line).
   - Minimal iconography: ▶ (play), ⏸ (pause), ⏹ (stop), ◀ (prev), ▶ (next).
   - Progress bar: 120 pixels wide × 2 pixels tall.
   - Clear background (black, 0x00) before redraw.

4. **Update Frequency:**
   - UI refresh: 10 Hz (100 ms) sufficient for human perception.
   - Event-driven updates: Immediate refresh on button press, track change.
   - No continuous polling loop (wastes power).

5. **Power Consumption:**
   - I2C @ 100 kHz, idle: ≈ 5 mA.
   - Display ON, typical content: ≈ 10–20 mA (depends on pixel density).
   - Sleep mode: 0xAE command reduces to ≈ 100 µA.

### Common Pitfalls
- **I2C Clock Stretching:** SSD1306 pulls SCL low if busy; code must respect this (IDF handles it).
- **Column/Page Addressing Confusion:** Easy to render upside-down or reversed; verify orientation.
- **Blocking I2C Writes:** If SSD1306 write blocks Bluetooth ISR, audio dropout. Use async writes.
- **No Dirty Tracking:** Full 1024-byte redraw every refresh @ 100 kHz I2C = ≈ 200 ms (too slow). Implement partial redraw.
- **Font Rendering Overhead:** Storing font bitmaps; pre-compute glyphs or use monospace fixed-width.

### Validation Criteria
- ✅ SSD1306 I2C address detected (0x3C).
- ✅ Initialization commands sequence successful.
- ✅ Pixels rendered correctly (draw test pattern: corners, center).
- ✅ Text rendered legibly (monospace font, >= 8 pt effective).
- ✅ Contrast adjustment functional (0x81 command varies brightness).
- ✅ Refresh @ 10 Hz causes no flicker (smooth animation on progress bar).
- ✅ Non-blocking writes (no audio dropout during display update).

---

## 6. EMBEDDED UI DESIGN FOR SMALL OLED (Flipper-Style)

### Definition
Designing minimal, functional monochrome UI for 128×64 pixel constraints inspired by Flipper Zero aesthetic.

### Micro Knowledge Brief
**Flipper Zero Design Principles:**
- **Monochrome Simplicity:** Black foreground, white background (or inverse).
- **Grid-Based Layout:** 8×8 or 16×16 grid; snap UI elements to grid.
- **Minimal Icons:** Simple 8×8 or 16×16 bitmaps (max 2–3 per screen).
- **Typography:** Monospace font, clear hierarchy (large title, small metadata).
- **No Animation (Unless Necessary):** Static renders; motion only on value change.
- **Information Density:** Show essential data only; hide metadata behind menus.

**UI State Machine:**
```
[Splash] → [Main Menu] → [Now Playing] → [Pause] → [Track List] → [Settings] → [Exit]
```

**Proposed Layout (Now Playing Screen):**
```
┌─────────────────────────────┐
│  Song Title (scrolling)     │  ← Line 0–7 px (title)
├─────────────────────────────┤
│  Artist Name                │  ← Line 8–15 px (metadata)
├─────────────────────────────┤
│  ▶ 02:34 / 04:12           │  ← Line 16–31 px (playback info)
├─────────────────────────────┤
│  ▓▓▓▓▓░░░░░░░░░░░░░░░░░░   │  ← Line 32–39 px (progress bar)
├─────────────────────────────┤
│                             │  ← Line 40–55 px (spacer)
├─────────────────────────────┤
│  [◀ Prev] [⏸ Pause] [▶ Nxt]│  ← Line 56–63 px (button hints)
└─────────────────────────────┘
```

**Font Sizing:**
- **Title:** 8pt (≈ 8 pixels height, fits 2 lines).
- **Metadata:** 6pt (fits 3 lines).
- **Buttons:** 6pt icons (8×8 px).

### Best Practices
1. **Layout Spacing:**
   - Top margin: 2 px (visual breathing room).
   - Inter-element gap: 4 px.
   - Bottom margin: 2 px.
   - Horizontal padding: 2 px left/right.

2. **Text Scrolling (Long Titles):**
   - Pre-compute scroll speed: 1 px per 50 ms = 20 px/sec.
   - Pause at edges for 1 second, then scroll.
   - Only scroll if text exceeds 100 pixels (≈ 12 chars @ 8pt).

3. **Progress Bar:**
   - Total width: 120 px (leave 4 px padding).
   - Filled: black (0x00), empty: white (0xFF).
   - Update every 100 ms (human-perceptible granularity).
   - Show elapsed time + total time in text (e.g., "2:34 / 4:12").

4. **Icon Design:**
   - Use monospace Unicode: ▶ (play), ⏸ (pause), ◀ (prev), ▶ (next).
   - Or define custom 8×8 bitmaps if Unicode unavailable.
   - Invert icons on dark background for contrast.

5. **Touch Feedback (Buttons):**
   - No visual feedback on button press (keeps design minimal).
   - Screen update on state change (play→pause, next track).

6. **Power Efficiency:**
   - Dim display after 30 sec inactivity (reduce contrast to 0x20).
   - Full sleep after 5 min (display OFF, 0xAE command).
   - Wake on button press.

### Common Pitfalls
- **Over-Decoration:** Borders, shadows, rounded corners = pixel waste on small display.
- **Poor Contrast:** Ensure foreground ≠ background; test on physical display.
- **Unreadable Font:** Avoid serif fonts; stick to monospace (5×7 or 6×8 minimum).
- **Flickering Text:** Don't redraw entire frame every refresh; only update changed regions.
- **Inadequate Button Hints:** Users won't know button functions without clear labeling.

### Validation Criteria
- ✅ Title and artist render legibly (no text truncation).
- ✅ Progress bar updates smoothly (no jitter).
- ✅ Button hints clearly visible (left, center, right align).
- ✅ Screen refresh @ 10 Hz causes no flicker.
- ✅ Scrolling text smooth and readable.
- ✅ Contrast sufficient for daylight viewing.
- ✅ Layout balanced (no excessive empty space).

---

## 7. BUTTON DEBOUNCING (NON-BLOCKING)

### Definition
Reliably detecting button state changes (press/release) while eliminating hardware contact noise using event-driven, non-blocking logic.

### Micro Knowledge Brief
**Debounce Physics:**
- Mechanical contacts bounce ≈ 5–20 ms after initial closure (switch-dependent).
- Reading GPIO during bounce gives erratic state transitions.
- Solution: Require stable state over debounce window (typically 20–50 ms).

**Debounce Algorithm (State Machine):**
```
Initial State: IDLE (stable, no change detected)

Event: GPIO edge detected (rising or falling)
  → Transition to WAIT state
  → Start debounce timer (20 ms)

In WAIT state:
  → If timer expires:
    → Sample GPIO; if consistent with expected direction:
      → Transition to PRESSED/RELEASED state
      → Queue press/release event
      → Return to IDLE
    → Else:
      → Debounce spurious; return to IDLE

Edge cases:
  → Multiple edges in WAIT window: Ignore, restart timer
  → User holds button: Single press event (state machine doesn't re-trigger)
```

**Non-Blocking Implementation:**
- Use hardware GPIO edge interrupt (rising, falling, or both).
- ISR records timestamp and queues debounce task.
- Debounce task (low-priority, can block) waits 20 ms, samples GPIO, posts event.
- Main loop polls event queue; processes press/release asynchronously.

**FreeRTOS Integration:**
- Use `xQueueSendFromISR()` in GPIO ISR.
- Use `xQueueReceive()` in debounce task.
- No busy-waiting; tasks sleep until events arrive.

### Best Practices
1. **Debounce Duration:**
   - Standard: 20 ms (covers most mechanical switches).
   - High-quality buttons: 10 ms.
   - Noisy environment: 50 ms.
   - **Selected:** 20 ms (proven industry standard).

2. **Event Queue Design:**
   - Queue capacity: 10 events (sufficient for 3 buttons, ~3 presses per 100 ms).
   - Event payload: button_id (0, 1, 2) + event_type (PRESS, RELEASE).
   - Drop oldest event if queue full (avoid memory bloat).

3. **Pull-Up/Pull-Down:**
   - Use internal GPIO pull-up (ESP32 supports).
   - Button = active-low (0 = pressed, 1 = released).
   - Prefer pull-up over pull-down (simpler wiring).

4. **Edge Detection:**
   - Use falling edge (GPIO → 0) to detect press (with pull-up).
   - Can also use both rising + falling edges (requires bistate logic).

5. **GPIO Configuration:**
   ```c
   gpio_config_t cfg = {
     .pin_bit_mask = (1ULL << btn1_pin) | (1ULL << btn2_pin) | (1ULL << btn3_pin),
     .mode = GPIO_MODE_INPUT,
     .pull_up_en = GPIO_PULLUP_ENABLE,
     .intr_type = GPIO_INTR_FALLING,
   };
   ```

### Common Pitfalls
- **Debounce Duration Too Short:** Bounces re-trigger ISR → multiple events per press.
- **Debounce Duration Too Long:** User perceives lag (> 50 ms noticeable).
- **Blocking in ISR:** ISR waits for GPIO stable → starves other interrupts.
- **No Edge Detection:** Polling GPIO in a loop → CPU waste.
- **Floating GPIO:** Without pull-up/down, noise → false triggers.

### Validation Criteria
- ✅ Single button press generates exactly 1 press event (no duplicates).
- ✅ Button release generates exactly 1 release event.
- ✅ Rapid pressing (< 100 ms apart) generates separate events (debounce doesn't suppress valid presses).
- ✅ No spurious events under EMI stress (test with nearby motor or WiFi).
- ✅ ISR latency < 5 µs (non-blocking queue post).
- ✅ Debounce task latency ≤ 30 ms end-to-end (press → event posted to main loop).

---

## 8. EVENT-DRIVEN FIRMWARE ARCHITECTURE

### Definition
Structuring firmware as independent, communicating modules using events/queues rather than polling or blocking calls.

### Micro Knowledge Brief
**Event-Driven vs. Polling:**
- **Polling:** Main loop checks states repeatedly (wastes CPU, power).
  ```c
  while (1) {
    if (button_pressed) handle_button();
    if (sd_card_ready) read_song();
    if (audio_ready) send_to_dac();
  }
  ```
- **Event-Driven:** Tasks block on events; resume only on state change.
  ```c
  xQueueReceive(event_queue, &event, portMAX_DELAY); // Block until event
  handle_event(event);
  ```

**Event Types:**
```
Button Events: BUTTON_PRESS(id), BUTTON_RELEASE(id)
Playback Events: PLAY, PAUSE, NEXT_TRACK, PREV_TRACK, SEEK
SD Events: FILE_LOADED, DIRECTORY_LISTED, CARD_ERROR
Bluetooth Events: CONNECTED, DISCONNECTED, AUDIO_STARTED, AUDIO_STOPPED
Display Events: REDRAW_UI, SCREEN_CHANGE
Audio Events: BUFFER_READY, UNDERRUN, SAMPLE_DECODED
```

**Task Hierarchy (FreeRTOS):**
```
Priority 25 (ISR) → GPIO ISR (GPIO event) + Bluetooth ISR (Bluetooth event)
Priority 24 → Audio Decode Task (MP3 decode, fill buffer)
Priority 23 → Bluetooth A2DP Feed Task (feed audio to BT stack)
Priority 22 → Display Refresh Task (render UI)
Priority 20 → Button Debounce Task (debounce GPIO events)
Priority 15 → Playback Control Task (interpret button events, state machine)
Priority 10 → SD Card Task (file I/O, directory listing)
Priority 5 → Idle Task (maintenance, heap stats)
```

**State Machine (Playback Control):**
```
         [STOPPED]
            ↕ (play button)
         [PLAYING] ↔ [PAUSED] (pause button)
            ↓ (next/prev button)
         [TRACK_CHANGED]
            ↓ (unload song, load next)
         [LOADING]
            ↓ (SD provides next track)
         [PLAYING]
```

### Best Practices
1. **Queue-Based Communication:**
   - Each task owns a queue (if it receives events).
   - Other tasks post to the queue; no direct function calls.
   - Promotes loose coupling, testability.

2. **Ownership & Responsibility:**
   - Audio Decode Task: Owns SD reader, MP3 decoder, output ring buffer.
   - Bluetooth A2DP Feed Task: Owns Bluetooth stack interaction, reads from ring buffer.
   - Display Task: Owns framebuffer, SSD1306 I2C driver.
   - Button Debounce Task: Owns GPIO ISR hook, button state machine.
   - Playback Control Task: Owns overall playback state, interprets user input.

3. **Data Structures:**
   - Define `event_t` union to minimize queue overhead:
     ```c
     typedef union {
       struct { uint8_t button_id; uint8_t event_type; } button;
       struct { uint8_t track_index; } playback;
       struct { char msg[32]; } error;
     } event_t;
     ```

4. **Error Propagation:**
   - Errors are events too (ERROR event with error code).
   - Display task picks up ERROR, shows message.
   - Playback control task stops playback on fatal error.

5. **Synchronization:**
   - Use queues (thread-safe).
   - Use mutexes only for shared buffers (ring buffer, framebuffer).
   - Avoid semaphores (queues are preferred in modern FreeRTOS).

### Common Pitfalls
- **Over-Synchronization:** Mutex protecting every variable → contention, deadlock risk.
- **Unbounded Queues:** Queue grows indefinitely if receiver starves → memory leak.
- **Event Ordering:** If Task A posts 2 events, Task B must process in order (queues guarantee this).
- **Lost Events:** Queue full, event dropped → user action ignored.
- **No Recovery:** On error, task waits forever → deadlock.

### Validation Criteria
- ✅ Button press → button_pressed event in playback control queue (≤ 30 ms).
- ✅ Playback control state machine transitions correctly (PLAYING → PAUSED → PLAYING).
- ✅ Audio decode task continuously fills ring buffer (no stalls > 100 ms).
- ✅ Display task refreshes UI without blocking audio decode.
- ✅ Error event (e.g., SD card removed) triggers graceful shutdown.
- ✅ No memory leaks (heap stable after 1 hour operation).

---

## 9. MEMORY & HEAP OPTIMIZATION ON ESP32

### Definition
Strategic memory allocation, fragmentation prevention, and profiling to maximize available RAM for audio buffering and firmware growth.

### Micro Knowledge Brief
**ESP32 Memory Layout:**
- **IRAM0:** 32 KB (fast, instruction cache).
- **IRAM1:** 32 KB (fast, D-cache).
- **DRAM:** 520 KB total (shared L3 cache + heap).
  - Bluetooth stack: ≈ 90 KB (fixed).
  - FreeRTOS kernel + task stacks: ≈ 50 KB.
  - **Available for application:** ≈ 300–350 KB.

**Heap Fragmentation:**
- Dynamic `malloc()` causes fragmentation (gaps between allocations).
- After several alloc/free cycles, even with total free space, no contiguous block available.
- ESP32 `heap_caps_` API mitigates this; prefer `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` for large buffers (if SPIRAM present).

**Audio Buffer Sizing:**
- Ring buffer for A2DP: 200 ms @ 44.1 kHz, 16-bit stereo = 352 KB.
- But with SPIRAM (4 MB external RAM), buffer can be much larger.
- Without SPIRAM: Pre-allocate 64 KB ring buffer (≈ 90 ms), require faster SD reads.

### Best Practices
1. **Pre-Allocation Strategy:**
   - Allocate all large buffers at boot (avoid fragmentation).
   - Use `heap_caps_malloc()` with explicit capability flags (DRAM, SPIRAM).
   - Never dynamically allocate in real-time loops.

2. **Profiling:**
   - Call `heap_caps_print_heap_stats()` periodically (e.g., every 10 sec).
   - Monitor free space; if drops below 20 KB, error log and reduce buffer size.
   - Use `heap_caps_get_largest_free_block()` to detect fragmentation.

3. **Task Stacks:**
   - Audio task: 4 KB (minimal work).
   - Decode task: 8 KB (decoder state + local buffers).
   - Display task: 4 KB.
   - SD task: 6 KB (FAT FS overhead).
   - Playback control: 4 KB.
   - **Total:** ≈ 26 KB (use `configTOTAL_HEAP_SIZE` to reserve).

4. **Static vs. Dynamic:**
   - Static allocation preferred (compile-time verification, no fragmentation).
   - Use `typedef struct { uint8_t buf[4096]; } ring_buffer_t;` in bss section.
   - Dynamic only for runtime sizing (e.g., SD directory listing).

5. **Memory Monitoring:**
   ```c
   extern uint32_t heap_caps_get_free_size(uint32_t caps);
   if (heap_caps_get_free_size(MALLOC_CAP_8BIT) < 20000) {
       ESP_LOGE(TAG, "Heap exhaustion risk");
   }
   ```

### Common Pitfalls
- **Allocating Buffers in ISR:** ISR runs with interrupts disabled; allocation can fail silently.
- **Not Checking Return Values:** `malloc()` returns NULL on failure; ignoring → null dereference.
- **Memory Leak in Error Path:** If error occurs after malloc, forgot to free → cumulative leak.
- **Excessive String Copying:** Each `strcpy()` allocates; use string refs or fixed buffers.
- **Not Using SPIRAM:** If 4 MB SPIRAM available, ignoring it leaves huge buffer potential unused.

### Validation Criteria
- ✅ Heap free space remains stable over 1 hour (no drift > 5%).
- ✅ Largest contiguous free block ≥ 20 KB at all times.
- ✅ Audio buffer size adequate (≥ 64 KB for smooth playback).
- ✅ No heap_caps_malloc() failures logged.
- ✅ Task stacks never overflow (use `uxTaskGetStackHighWaterMark()` to verify).

---

## 10. MODULAR C++ DESIGN FOR EMBEDDED SYSTEMS

### Definition
Structuring C++ code with clear separation of concerns, abstraction layers, and minimal coupling to support growth and testing.

### Micro Knowledge Brief
**Modular Architecture Principles:**
1. **Single Responsibility:** Each class handles one job (Decoder, Bluetooth, Display, etc.).
2. **Interface Contracts:** Pure virtual classes define expected behavior; implementations swap easily.
3. **Dependency Injection:** Pass dependencies (queues, buffers) to constructor, not globals.
4. **RAII (Resource Acquisition is Initialization):** Constructors allocate, destructors clean up (safer than manual cleanup).

**Module Organization:**
```
include/
  ├── audio_decoder.h      (virtual interface)
  ├── bluetooth_a2dp.h
  ├── display_ssd1306.h
  ├── button_handler.h
  ├── sd_card.h
  └── event_queue.h

src/
  ├── audio_decoder.cpp    (libhelix integration)
  ├── bluetooth_a2dp.cpp   (ESP-IDF wrapper)
  ├── display_ssd1306.cpp  (I2C driver)
  ├── button_handler.cpp   (debounce state machine)
  ├── sd_card.cpp          (FAT FS wrapper)
  ├── event_queue.cpp      (ring buffer implementation)
  └── main.cpp             (orchestration)
```

**Example: Audio Decoder Interface**
```cpp
class AudioDecoder {
public:
    virtual ~AudioDecoder() = default;
    virtual bool open(const char* filepath) = 0;
    virtual int decode_frame(int16_t* pcm_buffer, size_t max_samples) = 0;
    virtual void close() = 0;
    virtual uint32_t get_duration_ms() const = 0;
};

class MP3Decoder : public AudioDecoder {
private:
    HMP3Decoder handle;
    // ... implementation
};
```

### Best Practices
1. **Header Files:**
   - Declare interfaces in `.h` files.
   - Use `#pragma once` or include guards.
   - Minimize includes (reduce compile time).
   - Separate interface from implementation.

2. **Encapsulation:**
   - Private members (data, implementation details).
   - Public methods (behavior contract).
   - Protected virtual methods (for subclassing).

3. **Initialization:**
   ```cpp
   // Good: Constructor initializes all members
   Decoder::Decoder(Queue* event_q, RingBuffer* output)
       : event_queue_(event_q), output_buffer_(output), state_(IDLE) {}
   
   // Bad: Global state
   Queue g_event_queue;  // Globals = tight coupling
   ```

4. **Error Handling:**
   - Return error codes or exceptions (avoid silent failures).
   - Log errors with context (file, line, function).
   - Graceful degradation (don't crash on non-fatal errors).

5. **Testing Preparation:**
   - Inject dependencies (queues, buffers) → mockable in unit tests.
   - Minimal hardware-specific code in core logic.
   - Separate HAL (Hardware Abstraction Layer) from business logic.

### Common Pitfalls
- **God Classes:** Decoder class also handles Bluetooth, display → 1000+ lines, untestable.
- **Global State:** `extern int g_current_track;` → tight coupling, hard to trace.
- **Circular Dependencies:** `decoder.h` includes `bluetooth.h`, `bluetooth.h` includes `decoder.h` → won't compile.
- **Over-Abstraction:** 5 levels of inheritance for simple task → complexity without benefit.
- **No Error Handling:** `decoder->open(file);` might fail; no check → use-after-error.

### Validation Criteria
- ✅ Each module ≤ 500 lines (encourages focused design).
- ✅ Dependencies injected (testable; no globals).
- ✅ Interfaces (pure virtual) stable; implementations can change.
- ✅ Error paths tested (error codes propagated correctly).
- ✅ No circular includes (compiles cleanly).

---

## 11. EMBEDDED STATE MACHINES

### Definition
Modeling complex behaviors (playback, UI navigation, button debouncing) as deterministic finite state machines to ensure predictability and correctness.

### Micro Knowledge Brief
**FSM Components:**
- **States:** IDLE, PLAYING, PAUSED, LOADING, ERROR.
- **Transitions:** E.g., PLAYING + PAUSE_BUTTON → PAUSED.
- **Actions:** On entry/exit of state, or during transition (e.g., start audio decode on PLAYING entry).
- **Guard Conditions:** Transition only if condition met (e.g., FILE_LOADED before transitioning to PLAYING).

**Playback State Machine:**
```
     ┌────────────────────┐
     │     [IDLE]         │  Initial state
     │ on_enter: stop_dac │
     └────────┬───────────┘
              │
      user presses PLAY
              │
     ┌────────▼──────────┐
     │   [LOADING]       │  Loading file from SD
     │ on_enter: load_sd │
     └────────┬──────────┘
              │
           (success)
              │
     ┌────────▼──────────┐
     │   [PLAYING]       │  Decoding & streaming
     │ on_enter: start_a2dp
     │ action: feed_audio
     └────────┬──────────┘
         ┌────┴───────┐
    PAUSE    │      NEXT_TRACK
         │    │           │
     ┌────▼──┴──┐    ┌────▼──────┐
     │[PAUSED]  │    │[LOADING]  │ (jump to next track)
     └────▲──┬──┘    └───────────┘
    PAUSE  │ NEXT_TRACK / PREV
         └─────────────┘
```

**Button Debounce FSM:**
```
[IDLE] → [WAIT_STABLE] (on GPIO edge, start 20ms timer)
         → [PRESSED] (timer expires, GPIO stable low)
         → [IDLE] (on GPIO rising edge)

OR if GPIO bounces back during WAIT_STABLE:
         → [IDLE] (spurious, restart)
```

### Best Practices
1. **Explicit Transitions:**
   ```cpp
   enum State { IDLE, LOADING, PLAYING, PAUSED, ERROR };
   
   struct Transition {
       State from;
       Event event;
       State to;
       void (*action)(Context*);
       bool (*guard)(Context*);
   };
   
   // Explicit table
   Transition transitions[] = {
       { IDLE, EVENT_PLAY, LOADING, on_load_start, nullptr },
       { LOADING, EVENT_FILE_READY, PLAYING, on_play_start, is_audio_buffer_ready },
       { PLAYING, EVENT_PAUSE, PAUSED, on_pause, nullptr },
       // ... more transitions
   };
   ```

2. **Avoid Entanglement:**
   - Each state = simple, independent behavior.
   - Don't have states with overlapping logic.
   - Prefer "flat" FSM (all states at same level) over hierarchical (unless necessary).

3. **Debugging & Logging:**
   - Log state transitions (from/to).
   - Log events received.
   - Print state in debug output (helps diagnose hangs).

4. **Initialization:**
   - Clearly define initial state and entry conditions.
   - Verify no undefined transitions (catch all missing cases).

5. **Exception Handling:**
   - ERROR state for all failures.
   - From any state, ERROR_EVENT → ERROR.
   - ERROR state awaits user reset or auto-recovery timer.

### Common Pitfalls
- **State Explosion:** Too many states (15+) → hard to reason about.
- **Entangled Transitions:** One event causes multiple state changes → confusing.
- **Missing Guard Conditions:** Transition happens even if precondition fails (e.g., LOADING → PLAYING before file ready).
- **No Error States:** Assumption everything works → crashes on edge cases.
- **Implicit State (via variables):** State not explicitly tracked; determined by flags → hard to verify.

### Validation Criteria
- ✅ All state transitions tested (verify all paths in transition table).
- ✅ Guard conditions prevent invalid transitions (e.g., can't PLAY without song loaded).
- ✅ Error state reachable from all states (no path leads to hang).
- ✅ State transitions logged; verify sequence matches expectation.
- ✅ No race conditions (state changes atomic with event handling).

---

## 12. AUDIO BUFFERING STRATEGIES

### Definition
Designing efficient ring buffers to decouple audio producers (SD/MP3 decoder) from consumers (Bluetooth A2DP) and prevent underruns/overflows.

### Micro Knowledge Brief
**Ring Buffer Architecture:**
```
   Write Head → [empty | empty | empty | data | data | data | data | empty]
                                           ▲
                                    Read Head

Linear Memory (preallocated):
   [0] [1] [2] [3] [4] [5] [6] [7] [0] [1] [2] [3] ...
                                    (wrap-around)
```

**Size Calculation:**
- Real-time playback: 44.1 kHz, 16-bit, 2 channels = 176 bytes/10 ms.
- For 200 ms buffer: 176 × 20 = 3,520 bytes.
- Practical: Allocate 8 KB ring buffer (capacity ≈ 45 ms @ 44.1 kHz stereo).
- With 64 KB: ≈ 720 ms buffer (excellent headroom).

**Producer-Consumer Flow:**
```
[SD Card] (read_task) → [Ring Buffer (producer)] ← [Decode Task (fills)]
                                 ↓
                      [Ring Buffer (consumer)] ← [Bluetooth Task (reads)]
                                 ↓
                       [Bluetooth HW] → [Speaker]

Head = write position (where decoder writes next)
Tail = read position (where Bluetooth reads next)
Level = (head - tail) % size (approximate fullness)
```

**Underrun/Overflow Protection:**
- **Underrun:** If level drops to 0 (Bluetooth reads faster than decoder writes) → silence.
  - Solution: Pre-fill buffer 50 ms before playback.
  - Or: Gracefully handle empty buffer (output silence, log warning).
- **Overflow:** If level reaches capacity (decoder writes faster than Bluetooth reads).
  - Solution: Cap write; oldest data discarded (rare in Bluetooth A2DP, which is real-time).

### Best Practices
1. **Circular Buffer Implementation:**
   ```cpp
   class RingBuffer {
   private:
       uint8_t* buffer;
       size_t capacity;
       volatile size_t write_head, read_tail;
   
   public:
       size_t available() {
           return (write_head - read_tail) % capacity;
       }
       
       bool write(const uint8_t* data, size_t len) {
           if (available() + len > capacity) return false; // Overflow
           // Copy with wrap-around logic
           size_t space_to_end = capacity - write_head;
           memcpy(buffer + write_head, data, min(len, space_to_end));
           if (len > space_to_end) {
               memcpy(buffer, data + space_to_end, len - space_to_end);
           }
           write_head = (write_head + len) % capacity;
           return true;
       }
   };
   ```

2. **Thread-Safe Access:**
   - Write head updated by decoder task (low contention).
   - Read tail updated by Bluetooth task.
   - No mutex needed if reads/writes don't overlap (atomic on 32-bit).
   - If needed, use lightweight spinlock (not full mutex).

3. **Monitoring:**
   - Track buffer fullness over time (avg, min, max).
   - Trigger alarm if level < 10% (underrun imminent) or > 90% (overflow risk).
   - Log to assess optimal buffer size.

4. **Pre-Buffering Strategy:**
   - Before playback starts, fill buffer to 50% capacity.
   - Monitor during playback; if drops below 25%, pause and re-buffer (brief stall, better than dropout).

5. **Frame Alignment:**
   - Decoder outputs fixed-size frames (1152 samples stereo = 4.6 KB @ 44.1 kHz).
   - Write frames atomically (not partial frames) to avoid synchronization issues.

### Common Pitfalls
- **Off-by-One Errors:** Circular indexing (`%` operator) confusing; verify with unit tests.
- **Unbounded Writes:** Decoder writes > available space → data corruption.
- **No Pre-Buffering:** Start playback immediately → early underruns.
- **Contention:** Both decoder + Bluetooth tasks accessing same head/tail → race conditions.
- **Underestimating Buffer Size:** 8 KB insufficient if SD reads stall; causes frequent underruns.

### Validation Criteria
- ✅ Buffer size ≥ 64 KB (supports 45+ ms latency spikes).
- ✅ Pre-buffering achieved before playback (buffer ≥ 25% before A2DP starts).
- ✅ No buffer underruns logged during 1 hour playback (even with interruptions).
- ✅ Wrap-around logic verified (write beyond end-of-buffer, read from start correctly).
- ✅ Thread-safe (no corrupted PCM data, even under contention).

---

## 13. REPOSITORY STRUCTURING BEST PRACTICES

### Definition
Organizing source code, documentation, tools, and configurations to support reproducibility, collaboration, and maintenance.

### Micro Knowledge Brief
**Directory Hierarchy:**
```
esp32-bt-mp3-player/
├── README.md                    # Project overview
├── .gitignore                   # Ignore build artifacts
├── .github/workflows/           # CI/CD (optional, for testing)
├── docs/
│   ├── skills.md                # Engineering knowledge (this file)
│   ├── architecture.md          # System design
│   ├── ui-design.md             # Flipper-style UI layout
│   ├── wiring-guide.md          # Breadboard schematic
│   ├── pinout.md                # GPIO/I2C/SPI pin mapping
│   ├── hardware/
│   │   └── components-list.md   # Bill of materials
│   ├── firmware/
│   │   ├── build.md             # Compilation instructions
│   │   └── flashing.md          # Programming ESP32
│   ├── troubleshooting.md       # Debug guide
│   └── checkpoints.md           # Project milestones
├── firmware/
│   ├── platformio.ini           # Build config
│   ├── src/
│   │   ├── main.cpp             # Orchestration
│   │   ├── audio_decoder.cpp    # MP3 decoding
│   │   ├── bluetooth_a2dp.cpp   # BT stack
│   │   ├── display_ssd1306.cpp  # OLED driver
│   │   ├── button_handler.cpp   # Button debounce
│   │   └── sd_card.cpp          # SD I/O
│   ├── include/
│   │   ├── audio_decoder.h
│   │   ├── bluetooth_a2dp.h
│   │   ├── display_ssd1306.h
│   │   ├── button_handler.h
│   │   ├── sd_card.h
│   │   ├── event_queue.h
│   │   └── config.h             # Build config (pins, buffer sizes)
│   ├── lib/
│   │   └── libhelix/            # MP3 decoder (vendored or submodule)
│   └── test/
│       └── (unit tests, optional)
├── assets/
│   ├── schematic.png            # Hardware layout (visual)
│   ├── ui-mockup.png            # UI design (visual)
│   └── pinout-diagram.svg       # Pinout diagram
└── checkpoints/
    ├── 0-skills.md              # Completed checkpoint
    ├── 1-scaffold.md
    └── ... (milestone docs)
```

**Git Workflow:**
- Commit after each checkpoint (e.g., "Checkpoint 1 – Repo Scaffold Complete").
- Branch per feature if collaborating (not needed for solo).
- Tag releases (v0.1, v0.2, etc.).

### Best Practices
1. **Code Organization:**
   - One class per file (e.g., `audio_decoder.cpp` + `audio_decoder.h`).
   - Interfaces in `include/`; implementations in `src/`.
   - Minimize includes (reduce compile time).

2. **Configuration Management:**
   - All hardware pins in `include/config.h` (single source of truth).
   - Buffer sizes, timeouts, I2C addresses in `config.h`.
   - No magic numbers in code.

3. **Build System (PlatformIO):**
   - `platformio.ini` specifies board (esp32), framework (arduino or IDF), dependencies.
   - Build targets: `debug`, `release` (different optimization levels).
   - Test target: `test` (if unit tests added).

4. **Documentation:**
   - README.md: Overview, quick-start, dependencies.
   - architecture.md: System design, module interaction.
   - wiring-guide.md: Breadboard layout, pin connections.
   - troubleshooting.md: Common issues + solutions.

5. **Vendoring Dependencies:**
   - libhelix: Copy source into `lib/libhelix/` (avoid git submodules initially; complexity).
   - FreeRTOS: Included with ESP-IDF.
   - SSD1306 driver: Write custom (100 lines) or vendor if available.

### Common Pitfalls
- **No Config File:** Pin mappings scattered in code; changing board requires code search/replace.
- **Monolithic main.cpp:** 2000+ lines, hard to navigate.
- **No Documentation:** Future maintainer (or yourself in 6 months) lost.
- **Git Commits Too Frequent (or Too Rare):** Every line → noise; every day → hard to pinpoint bugs.
- **Missing .gitignore:** Build artifacts (`.build/`, `*.o`) committed → repo bloated.

### Validation Criteria
- ✅ Project builds cleanly from scratch (no undocumented dependencies).
- ✅ Pinout changes require only edit to `config.h` (no code changes).
- ✅ README.md sufficient to clone + build (no external wiki needed).
- ✅ Git history readable (commits have meaningful messages).
- ✅ No build artifacts in repository (checked .gitignore).

---

## SUMMARY: SKILL VALIDATION CHECKLIST

Before proceeding to Phase 1 (Repository Scaffold), validate the following:

| Skill Domain | Validation Criteria | Status |
|---|---|---|
| **ESP32 Architecture** | CPU freq confirmed, memory layout understood, peripherals mapped | ✓ Ready |
| **Bluetooth A2DP** | Ring buffer design documented, pairing strategy defined, codec selection (SBC) confirmed | ✓ Ready |
| **MP3 Decoding** | Decoder choice (libhelix), frame sync logic defined, error recovery planned | ✓ Ready |
| **SPI SD Card** | Initialization sequence documented, error handling defined, FAT32 integration planned | ✓ Ready |
| **I2C SSD1306** | Command set understood, framebuffer strategy (dirty-region tracking) confirmed, I2C clock rate chosen (100 kHz) | ✓ Ready |
| **Embedded UI Design** | Flipper-style layout documented, information hierarchy defined, refresh rate (10 Hz) chosen | ✓ Ready |
| **Button Debouncing** | Debounce FSM defined, duration (20 ms) chosen, event queue design (non-blocking) documented | ✓ Ready |
| **Event-Driven Architecture** | Task hierarchy and priorities defined, event types enumerated, queue communication documented | ✓ Ready |
| **Memory Optimization** | Heap layout understood, pre-allocation strategy defined, profiling approach documented | ✓ Ready |
| **Modular C++ Design** | Module breakdown defined, interfaces (pure virtual) identified, dependency injection planned | ✓ Ready |
| **State Machines** | Playback FSM documented, button debounce FSM documented, guard conditions defined | ✓ Ready |
| **Audio Buffering** | Ring buffer size calculated (64 KB target), pre-buffering strategy defined, thread-safety approach chosen | ✓ Ready |
| **Repository Structure** | Directory hierarchy planned, .gitignore defined, PlatformIO config outline drafted | ✓ Ready |

**PHASE 0 COMPLETE** when all skills validated. Proceed to Phase 1 – Repository Scaffold.

---

*Document Version: 1.0*  
*Last Updated: 2026-02-13*  
*Status: READY FOR PHASE 0 COMPLETION*
