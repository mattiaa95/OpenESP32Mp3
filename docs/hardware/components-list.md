# Bill of Materials (BOM)

Complete component list for ESP32-BT-MP3-Player breadboard prototype and eventual PCB.

---

## Microcontroller

| Part | Qty | Value | Supplier | Part# | Notes |
|------|-----|-------|----------|-------|-------|
| U1 | 1 | ESP32 DOIT DEVKIT V1 | Amazon, AliExpress | N/A | Dual-core 240 MHz, 520 KB RAM, 4 MB Flash |

**Alternative Options:**
- ESP32-S3 (higher perf, not needed)
- ESP32-C3 (too slow for this project)

---

## Storage & SD Card Interface

| Part | Qty | Value | Supplier | Part# | Notes |
|------|-----|-------|----------|-------|-------|
| U2 | 1 | microSD SPI Module | Amazon, AliExpress | N/A | Supports microSD, microSDHC, microSDXC |
| CARD1 | 1 | microSD Card 64GB | Amazon, Best Buy | Kingston, Samsung | UHS-I recommended (≥ 90 MB/s) |

**Notes:**
- SPI module typically includes level shifting (5V tolerant on input)
- Supports 3.3V logic from ESP32
- Can read/write at full SPI speed (20+ MHz)

---

## Display (OLED I2C)

| Part | Qty | Value | Supplier | Part# | Notes |
|------|-----|-------|----------|-------|-------|
| U3 | 1 | SSD1306 OLED 0.96" | Amazon, AliExpress | 128×64 monochrome | I2C address 0x3C default |

**Module Includes:**
- SSD1306 display controller
- 128×64 monochrome OLED panel
- I2C pull-up resistors (4.7 kΩ on SDA/SCL)
- On-board power bypass capacitors

---

## Input (Push Buttons)

| Part | Qty | Value | Supplier | Part# | Notes |
|------|-----|-------|----------|-------|-------|
| SW1, SW2, SW3 | 3 | 6mm Momentary Push Button | Amazon, Digi-Key | N/A | Normally open (NO) |

**Specifications:**
- Contact resistance: < 50 mΩ
- Bounce duration: 5–20 ms (debounced in firmware)
- Life: 1 million cycles typical
- Voltage rating: 24V DC (well over 3.3V)

**Mounting:**
- 2.54 mm pitch (breadboard compatible)
- Through-hole leads

---

## Passive Components

### Capacitors (Power Supply Decoupling & Filtering)

| Part | Qty | Value | Type | Voltage | Notes |
|------|-----|-------|------|---------|-------|
| C1 | 1 | 100 µF | Electrolytic | 16V | Bulk capacitor near ESP32 |
| C2 | 3 | 10 µF | Electrolytic | 16V | Near SD module, OLED, buttons |
| C3–C7 | 5 | 0.1 µF | Ceramic | 10V | High-frequency bypass (distributed) |

**Placement:**
- 100 µF: Across VCC/GND, adjacent to ESP32 power pins
- 10 µF: Across VCC/GND of each module
- 0.1 µF: Distributed along power rails, near each IC

### Resistors (Optional, for I2C Pull-ups if not on module)

| Part | Qty | Value | Type | Rating | Notes |
|------|-----|-------|------|--------|-------|
| R1, R2 | 2 | 4.7 kΩ | 1/4W | 5% Tolerance | Only needed if OLED lacks pull-ups |

**Note:** Most SSD1306 modules include 4.7 kΩ pull-ups on-board. Do NOT add external pull-ups if present (causes over-pulling and reduces open-drain drive strength).

---

## Connectors & Cabling

| Part | Qty | Value | Supplier | Notes |
|------|-----|-------|----------|-------|
| J1 | 1 | USB Micro-B Cable (5m) | Amazon | For power & programming |
| Breadboard | 1–2 | 830-pin or 2× 400-pin | Amazon | Solderless prototyping |
| Jumper Wires | ~50 | 22 AWG Male-Male | Amazon | Breadboard connections |

**Wire Gauge:** 22 AWG (0.64 mm²) standard for breadboard jumpers

---

## Power Supply

### Recommended

| Item | Voltage | Current | Notes |
|------|---------|---------|-------|
| USB 5V/1A Charger | 5V | 1A | Standard micro-USB charger (most common) |
| ESP32 Onboard Regulator | 3.3V | 500 mA | Built-in, regulated from USB 5V |

### Optional (for Battery Operation)

| Item | Voltage | Capacity | Notes |
|------|---------|----------|-------|
| LiPo Battery | 3.7V | 700–1000 mAh | Provides 3.5+ hours playback |
| TP4056 Charger Module | 5V input | N/A | USB charging for LiPo |
| Boost Converter | 3.7V → 5V | 500 mA | If using LiPo to power USB modules |

**Power Budget (estimated):**
- ESP32 idle: 30 mA
- ESP32 + Bluetooth: 150 mA
- SD Card (read): 150 mA
- OLED (on): 15 mA
- Total (playback): ~200 mA sustained

---

## Cost Breakdown (Approx, USD)

| Item | Qty | Unit Cost | Total |
|------|-----|-----------|-------|
| ESP32 DEVKIT V1 | 1 | $12 | $12 |
| microSD SPI Module | 1 | $3 | $3 |
| microSD Card 64GB | 1 | $15 | $15 |
| SSD1306 OLED 0.96" | 1 | $5 | $5 |
| 6mm Push Buttons | 3 | $0.50 | $1.50 |
| Capacitors (all) | ~10 | $0.10 | $1 |
| Breadboard + Jumpers | 1 kit | $8 | $8 |
| USB Cable & PSU | 1 | $5 | $5 |
| **Total** | — | — | **~$50** |

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-13  
**Status:** APPROVED FOR COMPONENT SOURCING
