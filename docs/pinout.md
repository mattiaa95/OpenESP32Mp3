# Hardware Pin Mapping & Wiring

## Pin Assignment Table

### ESP32 SPI Interface (SD Card)

| Signal | GPIO | Pin | Purpose | Notes |
|--------|------|-----|---------|-------|
| SPI_CLK | 18 | D18 | Serial Clock | 20 MHz (conservative) |
| SPI_MOSI | 23 | D23 | Master Out, Slave In | Data to SD card |
| SPI_MISO | 19 | D19 | Master In, Slave Out | Data from SD card |
| SPI_CS | 5 | D5 | Chip Select | Active-low |

### ESP32 I2C Interface (OLED Display)

| Signal | GPIO | Pin | Purpose | Notes |
|--------|------|-----|---------|-------|
| I2C_SDA | 21 | D21 | Serial Data | Pull-up 4.7 kΩ (on module) |
| I2C_SCL | 22 | D22 | Serial Clock | Pull-up 4.7 kΩ (on module) |

**I2C Address:** 0x3C (SSD1306 default)  
**I2C Speed:** 100 kHz (standard mode)

### GPIO Button Inputs (3× Push Buttons)

| Button | GPIO | Pin | Function | Logic |
|--------|------|-----|----------|-------|
| BTN_PREV | 26 | D26 | Previous track | Active-low (pull-up internal) |
| BTN_PLAY | 27 | D27 | Play/Pause | Active-low (pull-up internal) |
| BTN_NEXT | 14 | D14 | Next track | Active-low (pull-up internal) |

**Debounce Duration:** 20 ms (stable window)  
**Interrupt Type:** Falling edge (GPIO transitions from 1 → 0)  
**Pull-up:** Internal (GPIO_PULLUP_ENABLE)

### Power Rails

| Rail | Voltage | Current (max) | Source |
|------|---------|---------------|--------|
| VCC | +3.3 V | 500 mA | USB or external regulator |
| GND | 0 V | — | Common ground |

### Unused GPIO (Available for Expansion)

| GPIO | Pin | Status | Notes |
|------|-----|--------|-------|
| 0 | D0 | Available | Boot select (pull-up required) |
| 1 | D1 | UART TX | Serial debug (RX only) |
| 3 | D3 | UART RX | Serial debug |
| 4 | D4 | Available | — |
| 12 | D12 | Available | — |
| 13 | D13 | Available | — |
| 15 | D15 | Available | — |
| 16 | D16 | Available | — |
| 17 | D17 | Available | — |
| 25 | D25 | Available | DAC capable |
| 32 | D32 | Available | ADC capable |
| 33 | D33 | Available | ADC capable |
| 34 | D34 | Available | ADC capable (input-only) |
| 35 | D35 | Available | ADC capable (input-only) |
| 36 | D36 | Available | ADC capable (input-only) |
| 39 | D39 | Available | ADC capable (input-only) |

---

## Voltage Levels & Signal Integrity

### Logic Levels
- **Microcontroller:** 3.3 V (ESP32 native)
- **SD Card Module:** 3.3 V (standard for microSD)
- **OLED Display:** 3.3 V (I2C pull-ups on module)
- **Buttons:** 3.3 V pullup, 0 V when pressed

### Signal Integrity

| Signal | Max Frequency | Cable Length | Notes |
|--------|---------------|--------------|-------|
| SPI (CLK) | 20 MHz | < 30 cm | Short traces on breadboard; avoid long jumpers |
| SPI (MOSI) | 20 MHz | < 30 cm | — |
| SPI (MISO) | 20 MHz | < 30 cm | — |
| SPI (CS) | — | < 30 cm | Asserted low before clock, deasserted after |
| I2C (SDA/SCL) | 100 kHz | < 1 m | Pull-ups included on display module |
| GPIO (BTN) | DC | — | Bounces ~5–20 ms; debounced in firmware |

---

## Electrical Specifications

### Power Consumption Estimates

| Component | Mode | Current | Duration | Energy |
|-----------|------|---------|----------|--------|
| **ESP32** | Idle (240 MHz) | 30 mA | Continuous | — |
| **ESP32** | With Bluetooth | 150 mA | Playback | — |
| **SD Card** | Idle | 1 mA | Between reads | — |
| **SD Card** | Read @ 20 MHz | 150 mA | ~10 ms/block | — |
| **OLED Display** | On, typical | 15 mA | Playback | — |
| **OLED Display** | Off (sleep) | 0.1 mA | Paused | — |
| **Buttons** | Idle | 0 mA | — | — |
| **Total** | Playback (est) | **200 mA** | 1 hour | 720 mWh |

**Recommendation:** 5V/1A USB power supply (5W); 700 mAh LiPo battery for 3.5 hours portable.

---

## Grounding & Decoupling

### Power Distribution

```
USB/PSU (5V)
    ↓
[Voltage Regulator] → 3.3V
    ↓
[100 µF bulk cap]
    ↓
├─→ [10 µF cap near ESP32]
├─→ [10 µF cap near SD module]
├─→ [10 µF cap near OLED module]
└─→ [Common GND]
```

### Decoupling Capacitors

| Location | Capacitor | Purpose |
|----------|-----------|---------|
| ESP32 VCC (pins 2, 15) | 100 µF + 10 µF | Bulk + bypass |
| SD Module VCC | 10 µF | Local bypass |
| OLED Module VCC | 10 µF | Local bypass |
| All circuits | 0.1 µF (multiple) | High-frequency noise |

---

## I2C Pull-up Configuration

**Standard SSD1306 modules include pull-up resistors (4.7 kΩ) on SDA/SCL.**

If adding external pull-ups:
- Do NOT add if module has them (would over-pull and reduce drive strength)
- Use 4.7 kΩ resistors if needed
- Pull-up voltage = VCC (3.3V)

```
      VCC (3.3V)
        ↓
      [4.7kΩ] (if needed)
        ↓
GPIO_21 (SDA) ──→ [OLED Module] ─────→ GND
        ↓
      [Scope/Pullup]

GPIO_22 (SCL) ──→ [OLED Module] ─────→ GND
        ↓
      [Scope/Pullup]
```

---

## SPI Chip Select (CS) Timing

CS must be **asserted (low) before clock**, and **deasserted (high) after last clock bit**.

```
CS   ─────────┐    ┌──────────────────
              │    │
CLK  ──────┐ └────┐└─ ┌─┬─┬─┬─┬─┬─┬─┬─┐
           │      └──┬┴─┴─┴─┴─┴─┴─┴─┴─┘
           │          │
MOSI ──────┼─────────→ [DATA]
           │
MISO ←─────┼─────────[ DATA]

Legend:
  CS deasserted, 1–2 clock cycles → CS asserted low
  CS held low during 8-clock byte transfer
  After MISO read, CS deasserted high
  Minimum ~100 ns hold time
```

---

## Breadboard Wiring Checklist

- [ ] **Power Rails:** Red (3.3V), Black (GND) continuous on breadboard
- [ ] **ESP32:** Centered on breadboard; all pins accessible
- [ ] **SD Module:** SPI pins (CLK, MOSI, MISO, CS) connected
- [ ] **OLED Module:** I2C pins (SDA, SCL) connected
- [ ] **Buttons:** Each connected to GPIO + GND; 20 ms debounce in firmware
- [ ] **Decoupling:** 100 µF and 0.1 µF caps on VCC near each module
- [ ] **Ground:** Single common ground; no loops
- [ ] **Cable Lengths:** < 30 cm for SPI; < 1 m for I2C
- [ ] **Polarity:** VCC (red), GND (black) consistent
- [ ] **No Shorts:** Verify with multimeter (resistance between adjacent traces)

---

## Troubleshooting Electrical Issues

| Issue | Likely Cause | Fix |
|-------|--------------|-----|
| OLED doesn't show | I2C not responding | Check SDA/SCL connections; verify 0x3C address |
| SD card not detected | SPI CLK wrong speed | Verify 20 MHz in config.h; check CS assertion timing |
| Buttons not responsive | Floating GPIO | Verify internal pull-up enabled; check debounce window |
| Intermittent crashes | Noise on power rail | Add 100 µF bulk cap near ESP32; check ground loop |
| ESP32 won't boot | Insufficient power | Use 5V/1A supply; check voltage regulator output |
| I2C clock stretching | Display pulling SCL low | Allow ISR to timeout; check OLED module health |

---

## Design Notes

1. **SPI Frequency Conservative:** 20 MHz chosen for breadboard reliability; can increase to 40 MHz with proper layout (PCB).
2. **Internal Pull-ups:** All button GPIOs use ESP32's internal pull-ups (no external resistors needed).
3. **I2C Frequency Standard:** 100 kHz chosen for maximum compatibility; SSD1306 supports up to 400 kHz.
4. **Power Budget:** Assumes USB power; for battery operation, use 700+ mAh LiPo @ 3.3V.
5. **Future Expansion:** 20+ unused GPIO pins available for features (WiFi, UART debug, LED indicators).

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-13  
**Status:** READY FOR BREADBOARD PROTOTYPING
