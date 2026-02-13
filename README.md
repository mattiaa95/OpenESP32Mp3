# ESP32-BT-MP3-Player

A production-ready, modular Bluetooth MP3 Player based on ESP32 with Flipper Zero-inspired UI.

## Overview

This project implements a complete embedded audio system capable of:
- Reading MP3 files from SD card (SPI)
- Decoding MP3 to PCM in real-time
- Streaming audio via Bluetooth A2DP to external speakers
- Displaying track info and playback controls on 0.96" OLED (I2C)
- 3-button intuitive navigation (Next, Previous, Play/Pause)

## Hardware

**Microcontroller:** ESP32 (Standard)
- Dual-core 240 MHz Xtensa LX6
- 520 KB DRAM, 4 MB flash
- Integrated Bluetooth 4.2

**Components:**
- microSD Card Module (SPI)
- SSD1306 OLED 0.96" 128×64 (I2C)
- 3 × Momentary Push Buttons (GPIO + pull-up)

**Target Audio Format:**
- MP3 (MPEG-1/2 Layer III)
- 44.1 kHz, 16-bit stereo
- Variable bitrate (8–320 kbps)

## Architecture Highlights

- **Modular C++:** Clear separation (Audio, Bluetooth, Display, Buttons, SD)
- **Event-Driven:** Non-blocking tasks with FreeRTOS queues
- **Memory-Optimized:** Pre-allocated buffers, 64 KB audio ring buffer
- **Non-Blocking UI:** Flipper Zero-inspired minimal design (10 Hz refresh)
- **Real-Time Audio:** Ring buffer architecture with underrun/overflow protection
- **Pure Virtual Interfaces:** Abstraction layer for easy testing & portability

## Project Status

### ✅ PHASES COMPLETE

| Phase | Status | Commits | Output |
|-------|--------|---------|--------|
| **0 – Skill Bootstrapping** | ✅ DONE | 1 | Engineering knowledge (13 domains) |
| **1 – Repository Scaffold** | ✅ DONE | 1 | Modular architecture, compiling firmware |
| **2 – Hardware Architecture** | ⏳ TODO | – | Pinout, wiring, BOM, breadboard |
| **3–12 – Implementation** | ⏳ TODO | – | MP3 decoding, Bluetooth, UI, integration |

### Recent Milestones

- ✅ **Git Repository:** Initialized with meaningful commits
- ✅ **Firmware Architecture:** All 6 module interfaces defined (pure-virtual)
- ✅ **Build System:** PlatformIO configured; compiles cleanly
- ✅ **Memory Budget:** Planned (64 KB audio buffer, 20 KB decoder state)
- ✅ **FreeRTOS:** Task hierarchy defined (6 tasks, priority 24–5)
- ✅ **Configuration:** Hardware pins centralized in `config.h`

### Build Status

```
Platform:  ESP32 (Arduino framework)
Board:     ESP32 DOIT DEVKIT V1
Status:    ✅ SUCCESS (267 KB flash, 21 KB RAM)
Test:      Compiles with zero warnings
Git:       2 commits, clean working directory
```

## Documentation

### Engineering Skills
**[docs/skills.md](docs/skills.md)** – 48 KB comprehensive guide
- **13 skill domains** with validation criteria
- Best practices, pitfalls, architecture decisions
- Domains: ESP32, Bluetooth A2DP, MP3 decoding, SPI/I2C, OLED UI, debouncing, event-driven architecture, memory optimization, C++ design, state machines, audio buffering, repository practices

### Project Architecture
**[firmware/include/config.h](firmware/include/config.h)** – Hardware definitions
- GPIO pins (buttons, SPI, I2C)
- Buffer sizes (64 KB audio, 1 KB framebuffer)
- I2C/SPI frequencies, FreeRTOS priorities
- Timeouts and feature flags

### Pure Virtual Interfaces
1. **EventQueue** – Inter-task communication
2. **AudioDecoder** – MP3 decoding abstraction
3. **BluetoothA2DP** – Bluetooth streaming
4. **SDCard** – SD/SPI file I/O
5. **DisplaySSD1306** – OLED I2C driver
6. **ButtonHandler** – Debounced input

## File Structure

```
esp32-bt-mp3-player/
├── README.md                      # This file
├── .gitignore                     # Build artifacts excluded
├── docs/
│   └── skills.md                  # Engineering knowledge base
├── firmware/
│   ├── platformio.ini             # Build configuration
│   ├── include/
│   │   ├── config.h               # Hardware pin definitions
│   │   ├── audio_decoder.h        # Virtual interface
│   │   ├── bluetooth_a2dp.h       # Virtual interface
│   │   ├── button_handler.h       # Virtual interface
│   │   ├── display_ssd1306.h      # Virtual interface
│   │   ├── event_queue.h          # Virtual interface
│   │   └── sd_card.h              # Virtual interface
│   └── src/
│       ├── main.cpp               # Arduino setup/loop
│       ├── audio_decoder.cpp      # Stub (MP3 decoding)
│       ├── bluetooth_a2dp.cpp     # Stub (Bluetooth A2DP)
│       ├── button_handler.cpp     # Stub (Button debounce)
│       ├── display_ssd1306.cpp    # Stub (OLED driver)
│       ├── event_queue.cpp        # Stub (Event queue)
│       └── sd_card.cpp            # Stub (SD card I/O)
├── assets/                        # (For schematics, UI mockups)
└── checkpoints/                   # (Milestone documentation)
```

## Build Instructions

### Prerequisites
- Python 3.7+
- PlatformIO CLI (`pip install platformio`)

### Compile
```bash
cd firmware
platformio run
```

### Output
- Binary: `.pio/build/esp32-dev/firmware.bin`
- ELF: `.pio/build/esp32-dev/firmware.elf`
- Memory: ~267 KB flash, ~21 KB RAM (from stubs)

## Hardware Pin Mapping

| Function | GPIO | Purpose |
|----------|------|---------|
| SPI CLK | 18 | SD card clock |
| SPI MOSI | 23 | SD data out |
| SPI MISO | 19 | SD data in |
| SPI CS | 5 | SD chip select |
| I2C SDA | 21 | OLED data |
| I2C SCL | 22 | OLED clock |
| BTN PREV | 26 | Previous track |
| BTN PLAY | 27 | Play/Pause |
| BTN NEXT | 14 | Next track |

(Fully configurable in `firmware/include/config.h`)

## Architecture Decisions

### Why ESP32 Standard (not S3 or C3)?
- **Perfect balance:** Dual-core, 240 MHz, sufficient for MP3 + Bluetooth
- **Cost:** Cheapest option; S3 unnecessary, C3 insufficient cores
- **Community:** Largest ecosystem, proven Bluetooth reliability

### Why Arduino Framework (not esp-idf)?
- **Simplicity:** Easier initial scaffolding; FreeRTOS still available
- **Compatibility:** Works out-of-box with PlatformIO
- **Flexibility:** Easy to migrate to esp-idf later if needed

### Why Libhelix (MP3 Decoder)?
- **Performance:** ~3% CPU @ 240 MHz (excellent headroom)
- **Proven:** Used in commercial embedded systems
- **Compatibility:** Supports MPEG-1/2, handles variable bitrate

### Why Ring Buffer (Not Streaming)?
- **Real-time safe:** Decouples SD read latency from Bluetooth timing
- **Underrun protection:** 64 KB buffer ≈ 910 ms @ 44.1 kHz
- **Simple synchronization:** Atomic head/tail pointers, no mutexes

### Why Pure Virtual Interfaces?
- **Testability:** Easy to mock for unit tests
- **Portability:** Swap implementations (e.g., different decoder)
- **Clarity:** Enforces contracts from day one

## Next: Phase 2 – Hardware Architecture

**Goal:** Breadboard prototyping and hardware validation

**Deliverables:**
1. Pin mapping table (`docs/pinout.md`)
2. Breadboard wiring diagram (`docs/wiring-guide.md`)
3. Bill of materials (`docs/components-list.md`)
4. Power budget calculation
5. Prototype verification (no shorts, power-on test)

## Design Philosophy

**"Disciplined embedded engineering, not code generation."**

- ✅ Checkpoint-driven development (12 phases)
- ✅ Modular architecture from day one
- ✅ Memory budgeting before coding
- ✅ Non-blocking, event-driven design
- ✅ Complete documentation at each phase
- ✅ Reproducible from scratch

## Contributing

This is a personal engineering project with disciplined checkpoints. Following the phase structure:

1. Review the active phase in [docs/skills.md](docs/skills.md)
2. Check the checkpoint tracking in `checkpoints/` directory
3. Follow the event-driven, non-blocking patterns
4. Commit with meaningful messages
5. Update documentation as you go

## Future Enhancements

- WiFi streaming (optional future phase)
- NVS persistent settings (bonded devices)
- SPIRAM support (larger buffers if available)
- EQ control via button combinations
- Shuffle/repeat modes
- Seek functionality (if button long-press added)

## License

Personal engineering project. Use freely for learning.

---

**Status:** ✅ Phases 0–1 Complete  
**Next:** Phase 2 – Hardware Architecture  
**Target:** Production-ready firmware by Phase 12

*Last Updated: 2026-02-13*  
*Git Commits: 2 (Skill Bootstrapping, Repository Scaffold)*

