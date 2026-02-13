# Diagrama de Conexiones - Protoboard

## VISTA COMPLETA DE LA PROTOBOARD

```
┌────────────────────────────────────────────────────────────────────┐
│  PROTOBOARD - ESP32 MP3 Player                                     │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  +5V ══════════════════════════════════════════════════════════   │
│   ║                    (riel rojo - poder)                        │
│   ║                                                               │
│   ╠═ R 10k ════ GPIO 26 (PREV)    [botón presionado → GND]       │
│   ║                                                               │
│   ╠═ R 10k ════ GPIO 27 (PLAY)    [botón presionado → GND]       │
│   ║                                                               │
│   ╠═ R 10k ════ GPIO 14 (NEXT)    [botón presionado → GND]       │
│   ║                                                               │
│   ╠═ OLED SCL (GPIO 22)                                          │
│   ║                                                               │
│   ╠═ OLED SDA (GPIO 21)                                          │
│   ║                                                               │
│   ╠═ SD VCC                                                       │
│   ║                                                               │
│   ╠═ 3V3 ESP32                                                   │
│   ║                                                               │
│  GND ══════════════════════════════════════════════════════════   │
│   ║                    (riel negro - tierra)                     │
│   ║                                                               │
│   ╠═ GND de botones (3 cables)                                   │
│   ║                                                               │
│   ╠═ GND de OLED                                                 │
│   ║                                                               │
│   ╠═ GND de SD                                                   │
│   ║                                                               │
│   ╠═ GND de ESP32                                                │
│                                                                   │
└────────────────────────────────────────────────────────────────────┘
```

---

## CONEXIÓN DETALLADA DEL ESP32

```
         USB
         │
         VIN ←─ 5V (rojo)
         GND ←─ GND (negro)
         3V3 ───→ (Riel de 3.3V)
         
    ┌─────────────────┐
    │ ESP32 DevKit V1 │
    │                 │
    │ GPIO 18 ────→ CLK (SD Card)
    │ GPIO 23 ────→ MOSI
    │ GPIO 19 ────→ MISO
    │ GPIO 5  ────→ CS
    │                 │
    │ GPIO 21 ────→ SDA (OLED)
    │ GPIO 22 ────→ SCL
    │                 │
    │ GPIO 26 ────→ Botón Anterior
    │ GPIO 27 ────→ Botón Play
    │ GPIO 14 ────→ Botón Siguiente
    │                 │
    │ 3V3 ────────→ (pullups de botones)
    │ GND ────────→ (tierra común)
    │                 │
    └─────────────────┘
```

---

## TARJETA microSD (SPI)

```
Lector SD
  Pin VCC ──→ 3V3 (ESP32)
  Pin GND ──→ GND (ESP32)
  Pin CLK ──→ GPIO 18
  Pin MOSI ──→ GPIO 23
  Pin MISO ──→ GPIO 19
  Pin CS ──→ GPIO 5

Conexión en protoboard:
  VCC (rojo) del lector ──→ Carril 3V3
  GND (negro) del lector ──→ Carril GND
  CLK ──→ GPIO 18 (fila específica)
  MOSI ──→ GPIO 23
  MISO ──→ GPIO 19
  CS ──→ GPIO 5
```

---

## PANTALLA OLED (I2C)

```
SSD1306 OLED
  Pin VCC ──→ 3V3
  Pin GND ──→ GND
  Pin SDA ──→ GPIO 21
  Pin SCL ──→ GPIO 22

Notas:
• La pantalla YA tiene pull-ups internos
• NO añadas resistencias externas
• La dirección I2C es 0x3C (fija)
```

---

## BOTONES CON RESISTENCIAS PULL-UP

### Botón 1: ANTERIOR (GPIO 26)

```
      3V3 (rojo)
        │
      ┌─┴─┐
      │   │ 10kΩ
      │   │
      └─┬─┘
        │
      GPIO 26
        │
        ├─────→ [Botón]
        │       (2 patas)
        │         │
        │       (otra pata)
        │         │
        └─────────┴──→ GND (negro)

Cuando presionas el botón:
GPIO 26 = 0V

Cuando lo sueltas:
GPIO 26 = 3.3V (por resistor)
```

### Botón 2: PLAY (GPIO 27)
```
Igual al anterior, pero GPIO 27
```

### Botón 3: SIGUIENTE (GPIO 14)
```
Igual al anterior, pero GPIO 14
```

---

## ORDEN DE CONEXIÓN (PASO A PASO)

### 1️⃣ Alimentación
```
USB ──[Micro-B]──→ VIN ESP32
GND ──────────────→ GND ESP32
```

### 2️⃣ Rieles de Poder (Protoboard)
```
De ESP32 VIN → Carril rojo (+5V)
De ESP32 GND → Carril negro (GND)
```

### 3️⃣ Tarjeta SD (SPI)
```
SD VCC ──→ 3V3
SD GND ──→ GND
SD CLK ──→ GPIO 18
SD MOSI ──→ GPIO 23
SD MISO ──→ GPIO 19
SD CS ──→ GPIO 5
```

### 4️⃣ Pantalla OLED
```
OLED VCC ──→ 3V3
OLED GND ──→ GND
OLED SDA ──→ GPIO 21
OLED SCL ──→ GPIO 22
```

### 5️⃣ Botones + Resistores
Para CADA botón:
```
3V3 ──[Resistor 10kΩ]──→ GPIO (26/27/14)
      
Botón:
GPIO ──[Botón]──→ GND
```

---

## VERIFICACIÓN CON MULTÍMETRO

### Prueba 1: Tensión de Poder
```
Selector: V⎓ (voltios DC)
Rojo en carril +5V, Negro en carril GND
Lectura esperada: 5.0V ± 0.5V
```

### Prueba 2: Tierra Común
```
Selector: Ω (ohmios)
Rojo en cualquier GND, Negro en otro GND
Lectura esperada: < 1 Ω (casi 0)
```

### Prueba 3: Botón (antes de presionar)
```
Selector: V⎓
Rojo en GPIO 26, Negro en GND
Lectura esperada: 3.3V ± 0.2V
```

### Prueba 4: Botón (presionando)
```
Mismo selector
Mantén presionado el botón
Lectura esperada: 0V
```

---

## CHECKLIST VISUAL

```
Protoboard:
  [ ] Riel rojo conectado a 5V
  [ ] Riel negro conectado a GND
  [ ] Sin puentes accidentales entre rieles
  [ ] Cables bien insertados

ESP32:
  [ ] USB conectado
  [ ] Luz LED roja encendida (alimentación OK)

SD Card:
  [ ] Módulo insertado en protoboard
  [ ] Cables para CLK, MOSI, MISO, CS, VCC, GND
  [ ] tarjeta microSD dentro del lector

OLED:
  [ ] Pantalla encendida (se ve la pantalla)
  [ ] Cables para SDA, SCL, VCC, GND conectados
  [ ] Pantalla bien vertical/asegurada

Botones:
  [ ] 3 botones en diferentes secciones
  [ ] Cada uno con su resistor 10kΩ
  [ ] Todos los botones presionan bien (sin atascos)
  [ ] GND de todos en el carril negro
```

---

## IMAGEN REFERENCIA (ASCII)

```
                 ┌─────────────────────┐
                 │   ESP32 DevKit      │
                 │                     │
    ┌────────────┤ USB (poder+datos)   │
    │            │                     │
    ↓            │ GPIO 18 (SD CLK)    │
  [5V/GND]       │ GPIO 23 (SD MOSI)   │
                 │ GPIO 19 (SD MISO)   │
    Protoboard   │ GPIO 5 (SD CS)      │
  ┌────────────┐ │                     │
  │ Rojo: 5V   │ │ GPIO 21 (OLED SDA) │
  │ Negro: GND │ │ GPIO 22 (OLED SCL) │
  │            │ │                     │
  │ +5V ═══════╬─┤ VIN                 │
  │ GND ═══════╬─┤ GND                 │
  │ 3V3 ═══════╩─┤ 3V3                 │
  │            │ │                     │
  │  R 10k     │ │ GPIO 26 (PREV)      │
  │    │       │ │ GPIO 27 (PLAY)      │
  │  GPIO 26 ──┼─┤ GPIO 14 (NEXT)      │
  │    ▼       │ │                     │
  │ [botón]    │ └─────────────────────┘
  │    │       │
  │  GND       │
  │    │       │     Tarjeta SD
  │    └───────┼─────────[Lector]
  │            │
  │  GPIO 21──┼──→ OLED SDA
  │  GPIO 22──┼──→ OLED SCL
  │  GPIO 18──┼──→ SD CLK
  │  GPIO 23──┼──→ SD MOSI
  │  GPIO 19──┼──→ SD MISO
  │  GPIO 5───┼──→ SD CS
  │            │
  └────────────┘
```

---

*Diagrama de Conexiones - Enero 2025*
