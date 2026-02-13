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

**ESP32 (Standard)** – Dual-core 240 MHz Xtensa LX6
- DRAM: 520 KB
- Flash: 4 MB (external optional)
- Peripherals: SPI, I2C, GPIO, UART

**Components:**
- microSD Card Module (SPI)
- SSD1306 OLED 0.96" (I2C)
- 3 × Momentary Push Buttons (GPIO + pull-up)
- Bluetooth 4.2 (built-in to ESP32)

**Target Audio Format:**
- MP3 (MPEG-1/2 Layer III)
- 44.1 kHz, 16-bit stereo
- Variable bitrate (8–320 kbps)

## Architecture Highlights

- **Modular C++:** Clear separation (Audio, Bluetooth, Display, Buttons, SD)
- **Event-Driven:** Non-blocking tasks with FreeRTOS queues
- **Memory-Optimized:** Pre-allocated buffers, heap profiling
- **Non-Blocking UI:** Flipper Zero-inspired minimal design
- **Real-Time Audio:** Ring buffer architecture with overflow/underrun protection

## Checkpoints

This project follows strict checkpoint-driven development:

1. **Phase 0 – Skill Bootstrapping** ✓ COMPLETE
2. **Phase 1 – Repository Scaffold** (In Progress)
3. **Phase 2 – Hardware Architecture**
4. **Phase 3 – SD Card Reading**
5. **Phase 4 – MP3 Decoding**
6. **Phase 5 – Bluetooth A2DP Streaming**
7. **Phase 6 – OLED Driver**
8. **Phase 7 – Flipper-Style UI**
9. **Phase 8 – Button Handling**
10. **Phase 9 – Full System Integration**
11. **Phase 10 – End-to-End Validation**
12. **Phase 11 – Documentation Finalization**
13. **Phase 12 – Production Ready**

## Documentation

- **[Skills & Knowledge](docs/skills.md)** – Engineering domain knowledge
- **[Architecture](docs/architecture.md)** – System design (coming soon)
- **[Wiring Guide](docs/wiring-guide.md)** – Breadboard layout (coming soon)
- **[UI Design](docs/ui-design.md)** – Flipper-style interface (coming soon)
- **[Troubleshooting](docs/troubleshooting.md)** – Debug guide (coming soon)

## Build & Flash

```bash
cd firmware
platformio run --target upload
```

See [docs/firmware/build.md](docs/firmware/build.md) for detailed instructions.

## Status

**Current Phase:** Phase 0 – Skill Bootstrapping (COMPLETE)

All engineering skill domains documented and validated. Ready to proceed to Phase 1.
