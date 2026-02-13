# Breadboard Wiring Guide

Complete step-by-step instructions for prototyping the ESP32-BT-MP3-Player on a breadboard.

---

## Materials Checklist

### Electronics
- [ ] ESP32 DOIT DEVKIT V1 microcontroller
- [ ] microSD card module (SPI interface)
- [ ] SSD1306 OLED display 0.96" (I2C interface)
- [ ] 3× push buttons (momentary, 6mm)
- [ ] 1× breadboard (830-pin minimum, or two 400-pin boards)
- [ ] Jumper wires (male-male, ~40 pieces)
- [ ] USB cable (USB-A to Micro-B) for power & programming
- [ ] Resistors (optional): 4.7 kΩ × 2 (if adding external I2C pull-ups)
- [ ] Capacitors: 100 µF × 1, 10 µF × 3, 0.1 µF × 5

### Tools
- [ ] Multimeter (verify voltages, check for shorts)
- [ ] Helping hands / PCB vise (optional, for stability)
- [ ] Pen and paper (document connections as you go)

---

## Step 1: Prepare the Breadboard

1. **Layout the breadboard** horizontally in front of you
2. **Identify power rails** (typically outer columns, marked + and −)
3. **Connect main power:**
   - Plug USB cable into ESP32 (Micro-B port)
   - Connect one breadboard's **positive rail (red)** across the top
   - Connect one breadboard's **negative rail (black)** across the bottom
   - **For second breadboard:** Bridge power rails with jumper wires (if using two boards)

```
┌─────────────────────────────┬─────────────────────────────┐
│ POWER RAIL (Red = +3.3V)    │ POWER RAIL (Red = +3.3V)    │
├─────────────────────────────┼─────────────────────────────┤
│                             │                             │
│  [Space for modules]        │  [Space for modules]        │
│                             │                             │
├─────────────────────────────┼─────────────────────────────┤
│ GND RAIL (Black = 0V)       │ GND RAIL (Black = 0V)       │
└─────────────────────────────┴─────────────────────────────┘
         Board 1                      Board 2
```

---

## Step 2: Mount ESP32

1. **Position ESP32** in the center of breadboard (straddling center channel)
   - Left side: GPIO 0–22
   - Right side: GPIO 23–39
2. **Insert each pin** into a breadboard hole (press firmly)
3. **Verify alignment:** All pins should be level; no bent pins

```
         ┌──────────────┐
      ┌──┤ MICRO-B USB  ├──┐
      │  │ Port         │  │
      │  └──────────────┘  │
      │                    │
  0   1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22
  └──┘ ├─────────────────────────────────────────────────────────────┤
     └───────────────────────────────────────────────────────────────┘
  23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39
  └──┘ └─────────────────────────────────────────────────┤
```

---

## Step 3: Connect Power & Ground

### From USB (via ESP32)

ESP32 provides:
- **3.3V_OUT** (pin on ESP32; labeled "3V3") → **Breadboard +3.3V rail** (red)
- **GND** (multiple pins on ESP32) → **Breadboard GND rail** (black)

**Connections:**
```
ESP32 Pin          Breadboard
─────────────────────────────
GND (pin 1/15/38) → GND rail (black)
3.3V_OUT (if avail) → VCC rail (red)
```

**Recommended:** If ESP32 doesn't have dedicated 3.3V out, power modules directly:
- Solder 5V from USB, pass through low-dropout regulator to 3.3V
- Or use breadboard power supply module (USB-powered)

**Add Decoupling Capacitors:**
1. **100 µF capacitor** across VCC and GND (near ESP32)
   - **+** lead → VCC rail
   - **−** lead → GND rail
2. **0.1 µF capacitor** across VCC/GND in 3 more locations (near SD module, OLED, buttons)

```
Capacitor symbol (electrolytic, "|" is negative):
        ─┬─       ─┬─
     +  │ │ −   +  │ │ −
        ─┴─       ─┴─
       100µF      0.1µF
```

---

## Step 4: Connect SD Card Module (SPI)

### Pinout (typical microSD SPI module)

| Module Pin | Signal | → ESP32 GPIO |
|------------|--------|--------------|
| CLK | SPI Clock | 18 (D18) |
| MOSI | Data Out | 23 (D23) |
| MISO | Data In | 19 (D19) |
| CS | Chip Select | 5 (D5) |
| VCC | +3.3V | Power rail (red) |
| GND | Ground | GND rail (black) |

### Wiring Steps

1. **CLK (breadboard pin for GPIO 18)** → **Jumper** → **SD Module CLK**
2. **MOSI (GPIO 23)** → **Jumper** → **SD Module MOSI**
3. **MISO (GPIO 19)** → **Jumper** → **SD Module MISO**
4. **CS (GPIO 5)** → **Jumper** → **SD Module CS**
5. **Power rail** → **Jumper** → **SD Module VCC**
6. **GND rail** → **Jumper** → **SD Module GND**
7. **Add 10 µF capacitor** across SD Module VCC/GND

```
Breadboard (top view):
     D18(CLK) ──→ [SD Module]
     D23(MOSI)──→
     D19(MISO)←──
     D5(CS)  ──→
     VCC────────→ VCC
     GND────────→ GND
```

### Verification
- [ ] SD module has 6 pins connected
- [ ] No jumper crossing (organize below modules)
- [ ] CLK frequency set to 20 MHz (in config.h)

---

## Step 5: Connect OLED Display (I2C)

### Pinout (typical SSD1306 I2C module)

| Module Pin | Signal | → ESP32 GPIO |
|------------|--------|--------------|
| SDA | Serial Data | 21 (D21) |
| SCL | Serial Clock | 22 (D22) |
| VCC | +3.3V | Power rail (red) |
| GND | Ground | GND rail (black) |

### Wiring Steps

1. **SDA (GPIO 21)** → **Jumper** → **OLED Module SDA**
2. **SCL (GPIO 22)** → **Jumper** → **OLED Module SCL**
3. **VCC rail** → **Jumper** → **OLED Module VCC**
4. **GND rail** → **Jumper** → **OLED Module GND**
5. **Add 10 µF capacitor** across OLED VCC/GND

**Note:** Most OLED modules include 4.7 kΩ pull-up resistors on SDA/SCL. Do NOT add external pull-ups if present (check module datasheet).

```
Breadboard (top view):
     D21(SDA)──→ [OLED Module]
     D22(SCL)──→
     VCC────────→ VCC
     GND────────→ GND
               (Pull-ups on module)
```

### Verification
- [ ] OLED module has 4 pins connected
- [ ] Pull-up resistors present on module
- [ ] I2C address is 0x3C (default; verify with code)

---

## Step 6: Connect Push Buttons

### Button Configuration

Each button has **2 pins:**
- **Pin 1:** Connected to **GPIO pin** (with internal pull-up)
- **Pin 2:** Connected to **GND** (active-low)

| Button | GPIO | Breadboard Position |
|--------|------|---------------------|
| PREV | 26 (D26) | Row 26 |
| PLAY | 27 (D27) | Row 27 |
| NEXT | 14 (D14) | Row 14 |

### Wiring Steps

1. **Button PREV:**
   - **One leg** → Jumper → **GPIO 26 (D26)**
   - **Other leg** → Jumper → **GND rail**

2. **Button PLAY:**
   - **One leg** → Jumper → **GPIO 27 (D27)**
   - **Other leg** → Jumper → **GND rail**

3. **Button NEXT:**
   - **One leg** → Jumper → **GPIO 14 (D14)**
   - **Other leg** → Jumper → **GND rail**

```
Button symbol:
   ├─●─┤  (pin 1)
   │   │
   │   │  When pressed:
   │   │  pin 1 connects to pin 2
   │   │
   ├───┤  (pin 2)

Breadboard layout:
   GPIO 26 ──→ [Button PREV] ←── GND
   GPIO 27 ──→ [Button PLAY] ←── GND
   GPIO 14 ──→ [Button NEXT] ←── GND
```

### Verification
- [ ] Each button has 2 connections (GPIO + GND)
- [ ] Internal pull-ups enabled in firmware (GPIO_PULLUP_ENABLE)
- [ ] Debounce set to 20 ms in config.h

---

## Step 7: Final Verification

### Visual Inspection Checklist

- [ ] **Power:** Red rail connected to 3.3V, black rail to GND
- [ ] **ESP32:** Centered, all pins inserted straight (no bent pins)
- [ ] **SD Module:** 6 pins connected (CLK, MOSI, MISO, CS, VCC, GND)
- [ ] **OLED Module:** 4 pins connected (SDA, SCL, VCC, GND)
- [ ] **Buttons:** 3 buttons, each with GPIO + GND connection
- [ ] **Capacitors:** 100 µF near ESP32, 10 µF near SD/OLED, 0.1 µF distributed
- [ ] **No Shorts:** Adjacent jumpers not touching; trace spacing adequate
- [ ] **Cable Neatness:** Jumpers organized below/beside modules

### Electrical Testing (Multimeter)

1. **Power Voltage:**
   - Probe **+** on VCC rail, **−** on GND rail
   - Reading should be **3.3V ± 0.1V**

2. **Check for Shorts:**
   - Switch multimeter to **Resistance (Ω) mode**
   - Probe between **VCC and GND rails**
   - Reading should be **> 10 kΩ** (if < 1 kΩ, short detected)

3. **Button Continuity:**
   - Probe between **GPIO pin and GND**
   - **Unpressed:** Resistance should be **> 100 kΩ** (open)
   - **Pressed:** Resistance should be **< 100 Ω** (closed)

4. **I2C Pull-ups (Check with Oscilloscope if available):**
   - SDA/SCL should idle at **3.3V** (high)
   - Should pull to **0V** when driven (test with i2cdetect)

---

## Step 8: Connect to Computer & Flash Firmware

### USB Connection

1. **Connect Micro-B USB cable** to ESP32 (Micro-B port)
2. **Plug USB into computer**
3. **LED on ESP32 lights up** (red or blue, depends on board version)
4. **On Mac:** Device appears as `/dev/ttyUSB0` or `/dev/cu.usbserial-*`
5. **Verify device:** `ls /dev/tty* | grep USB` (on Mac/Linux)

### Flash Firmware

```bash
cd firmware
platformio run --target upload
```

### Monitor Output

```bash
platformio device monitor --baud 115200
```

Expected output:
```
=== ESP32 Bluetooth MP3 Player ===
Firmware starting...
All tasks started
Main loop heartbeat
Main loop heartbeat
...
```

---

## Step 9: Functional Testing

### Test Sequence

1. **Power-On Test**
   - [ ] ESP32 boots (LED indicator)
   - [ ] Serial output appears (115200 baud)
   - [ ] No crashes or resets

2. **SD Card Test** (Phase 3)
   - [ ] Insert microSD card (with MP3 files in root)
   - [ ] Read directory via serial command
   - [ ] Verify file listing

3. **OLED Test** (Phase 6–7)
   - [ ] Display powers on
   - [ ] Test pattern visible (corners, center)
   - [ ] Text renders clearly

4. **Button Test** (Phase 8)
   - [ ] Press each button
   - [ ] Monitor shows button events (serial log)
   - [ ] No false triggers

5. **Bluetooth Test** (Phase 5)
   - [ ] ESP32 visible on Bluetooth device list
   - [ ] Pair with external speaker
   - [ ] Audio streams (once MP3 decoding enabled in Phase 4)

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| No power | Broken USB cable or ESP32 port | Try different USB cable; test power with multimeter |
| ESP32 won't upload | Driver missing or wrong COM port | Install CH340 drivers; verify COM port in platformio.ini |
| OLED black/no display | I2C not connected or wrong address | Check SDA/SCL; verify address 0x3C in firmware |
| SD card not detected | SPI pin mismatch or CS not asserted | Verify pin assignments match config.h; check CS timing |
| Button always pressed | GPIO short to GND | Check for crossed jumpers; test with multimeter |
| Intermittent crashes | Power noise or insufficient decoupling | Add more 0.1 µF capacitors; use USB supply with ferrite choke |

---

## Next Steps

Once breadboard prototype verified:
1. Proceed to **Phase 3 – SD Card Reading**
2. Load test MP3 file onto microSD card
3. Run directory listing test
4. Verify file read speed (expect ≥ 1 MB/sec @ 20 MHz SPI)

---

**Document Version:** 1.0  
**Last Updated:** 2026-02-13  
**Status:** READY FOR BREADBOARD ASSEMBLY
