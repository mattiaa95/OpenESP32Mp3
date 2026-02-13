# ESP32 MP3 Player - Guía Rápida en Español

## PARTE 1: CONEXIONES EN LA PROTOBOARD

### Materiales Necesarios
- ESP32 DevKit V1
- Tarjeta microSD + lector SPI
- Pantalla OLED SSD1306 (0.96", I2C)
- 3 botones
- 3 resistencias 10kΩ
- Cable USB (micro-B)
- Protoboard
- Cables de conexión

---

## PASO 1: CONEXIONES DE ALIMENTACIÓN

```
Puerto USB (5V)
    ↓
    ├─→ ESP32 pin VIN (rojo)
    └─→ GND (negro)

En la protoboard (riel de poder):
    5V ←────→ Rojo (carril superior)
    GND ←────→ Negro (carril inferior)
```

**Verificar con multímetro:**
- Rojo a Negro = 5V
- Sin cortocircuitos entre carriles

---

## PASO 2: TARJETA microSD (SPI)

| Componente | Pin ESP32 |
|-----------|-----------|
| CLK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| CS | GPIO 5 |
| 3.3V | 3V3 |
| GND | GND |

**En la protoboard:**
```
Lector SD:
  CLK ──→ GPIO 18
  MOSI ──→ GPIO 23
  MISO ──→ GPIO 19
  CS ──→ GPIO 5
  VCC ──→ 3V3
  GND ──→ GND
```

---

## PASO 3: PANTALLA OLED I2C

| Componente | Pin ESP32 |
|-----------|-----------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 3V3 |
| GND | GND |

**En la protoboard:**
```
Pantalla OLED:
  SDA ──→ GPIO 21
  SCL ──→ GPIO 22
  VCC ──→ 3V3
  GND ──→ GND

IMPORTANTE: La pantalla ya tiene resistencias pull-up internas.
NO añadas resistencias externas.
```

---

## PASO 4: BOTONES (3 piezas)

**Para CADA botón:**

| Botón | GPIO | Resistor 10kΩ |
|-------|------|----------------|
| ANTERIOR | 26 | 26 → 3V3 |
| PLAY | 27 | 27 → 3V3 |
| SIGUIENTE | 14 | 14 → 3V3 |

**Conexión de cada botón:**
```
Botón (2 patas):
  Pata 1 ──→ GPIO (26, 27 o 14)
  Pata 2 ──→ GND

Resistor 10kΩ (pull-up):
  Un extremo ──→ GPIO (mismo que botón)
  Otro extremo ──→ 3V3 (no el botón)

Cuando presionas: GPIO baja a 0V
Cuando sueltas: GPIO sube a 3.3V (por resistor)
```

---

## PASO 5: VERIFICAR CONEXIONES

**Multímetro (continuidad):**
1. GPIO 18 → Pin CLK del lector SD
2. GPIO 21 → SDA de OLED
3. GPIO 26 → Pin del botón
4. GND → Todos conectados al carril negro
5. 3V3 → Resistores y 3V3 de módulos

**Visual:**
- ✅ Sin cables sueltos
- ✅ Sin puentes accidentales
- ✅ Componentes firmes en protoboard
- ✅ USB conectado al ESP32

---

## PARTE 2: INSTALAR SOFTWARE

### Opción A: Arduino IDE (MÁS FÁCIL)

#### Paso 1: Descargar Arduino IDE
https://www.arduino.cc/en/software

#### Paso 2: Agregar ESP32
1. Abre Arduino IDE
2. Archivo → Preferencias
3. En "URLs de Gestor de tarjetas adicionales" pega:
```
https://dl.espressif.com/dl/package_esp32_index.json
```
4. Aceptar
5. Herramientas → Placa → Gestor de tarjetas
6. Busca "ESP32" de Espressif
7. Instala la versión más reciente

#### Paso 3: Configurar Placa
Herramientas:
- Placa: "ESP32 Dev Module"
- Puerto: COM3 (o el que muestre tu ESP32)
- Velocidad carga: 460800

#### Paso 4: Copiar Código
Descarga el código del proyecto:
```bash
git clone https://github.com/usuario/esp32-bt-mp3-player.git
```

O copia el archivo `firmware/src/main.cpp` directamente.

#### Paso 5: Subir Código
1. Pega el código en Arduino IDE
2. Botón Verificar (✓) - Compila sin errores
3. Botón Subir (→) - Carga al ESP32

**Esperado en 10-15 segundos:**
```
Leaving... 
Hard resetting via RTS pin...
```

---

### Opción B: PlatformIO (RECOMENDADO)

#### Paso 1: Instalar VS Code
https://code.visualstudio.com/

#### Paso 2: Instalar PlatformIO
1. Abre VS Code
2. Extensions (Ctrl+Shift+X)
3. Busca "PlatformIO IDE"
4. Instala

#### Paso 3: Abrir Proyecto
1. PlatformIO Home → Open Project
2. Selecciona la carpeta `esp32-bt-mp3-player/firmware`

#### Paso 4: Compilar y Subir
```bash
# En terminal de PlatformIO:
platformio run --target upload
```

O usa los botones:
- Build (compile)
- Upload (carga al ESP32)

---

## PARTE 3: PROBAR EL SISTEMA

### Preparar microSD
1. Formatea la tarjeta como FAT32
2. Copia archivos MP3 (3-5 archivos de prueba)
3. Inserta en el lector

### Monitor Serial
Para ver mensajes de depuración:

**Arduino IDE:**
- Herramientas → Monitor Serial (115200 baud)

**PlatformIO:**
```bash
platformio device monitor
```

### Esperado en el Boot
```
╔════════════════════════════════════════════════════════════╗
║       ESP32 Bluetooth MP3 Player - Starting Up             ║
╚════════════════════════════════════════════════════════════╝
Build: Feb 13 2025 17:00:00
Free heap: 235KB

[INIT] Creating event queue...
[INIT] Initializing display...
[OLED] Initialized successfully
[INIT] Initializing SD card...
[SD] Found 3 MP3 files
  [0] cancion1.mp3
  [1] cancion2.mp3
  [2] cancion3.mp3

[INIT] Initialization complete
════════════════════════════════════════════════════════════
```

---

## PARTE 4: PRUEBAS RÁPIDAS

### Prueba 1: Pantalla
- ✅ Aparece "Ready!" en la OLED después de 2 segundos
- ✅ Sin flicker, sin líneas raras

### Prueba 2: Botones
Presiona cada botón y mira el Monitor Serial:
```
[BTN] PREV PRESS    ← Botón anterior
[BTN] PLAY PRESS    ← Botón play
[BTN] NEXT PRESS    ← Botón siguiente
```

### Prueba 3: Tarjeta SD
Mira el Monitor Serial:
```
[SD] Found 3 MP3 files     ← Detectó los archivos
[SD] Listed 3 MP3 files    ← Los leyó correctamente
```

---

## SOLUCIONES RÁPIDAS

### "Puerto COM no aparece"
- Instala driver CH340: https://www.wch.cn/downloads/ch341ser_exe.html
- Reinicia PC

### "OLED no inicializa"
```
[OLED] FATAL: SSD1306 not found on I2C bus
```
Solución:
- Verifica GPIO 21 (SDA) y GPIO 22 (SCL)
- Cables conectados bien
- Pantalla tiene poder (voltaje en VCC/GND)

### "SD Card no detecta"
```
[SD] Failed to initialize SD card
```
Solución:
- En `config.h` cambia:
  ```cpp
  #define SPI_CLOCK_FREQ 10  // Reduce de 20 a 10 MHz
  ```
- Recompila y sube

### "Botones no responden"
Verifica con multímetro:
- Botón presionado: debe leer 0V
- Botón suelto: debe leer 3.3V

---

## DIAGRAMA FINAL

```
                    ESP32 DevKit
                 ┌──────────────┐
    USB ──────→  │   VIN GND    │
                 │              │
    GPIO 18 ←───→│ SD_CLK       │
    GPIO 23 ←───→│ SD_MOSI      │
    GPIO 19 ←───→│ SD_MISO      │
    GPIO 5  ←───→│ SD_CS        │
                 │              │
    GPIO 21 ←───→│ OLED_SDA     │
    GPIO 22 ←───→│ OLED_SCL     │
                 │              │
    GPIO 26 ←───→│ BTN_PREV     │
    GPIO 27 ←───→│ BTN_PLAY     │
    GPIO 14 ←───→│ BTN_NEXT     │
                 │              │
    3V3 ─────────│ 3V3          │
    GND ─────────│ GND          │
                 └──────────────┘
```

---

## CHECKLIST FINAL

- [ ] Cables USB conectados
- [ ] Resistencias 10kΩ en botones (3)
- [ ] GPIO 18/23/19/5 conectados a SD
- [ ] GPIO 21/22 conectados a OLED
- [ ] GPIO 26/27/14 conectados a botones
- [ ] Todos los GND conectados
- [ ] Todos los 3V3 conectados (excepto botones)
- [ ] Multímetro: 5V en carril rojo
- [ ] Multímetro: 0V en carril negro
- [ ] Arduino IDE con ESP32 instalado
- [ ] Puerto COM seleccionado
- [ ] Código compilado sin errores
- [ ] Código subido exitosamente
- [ ] Monitor Serial muestra "Ready!"
- [ ] microSD formateada FAT32
- [ ] MP3s copiados en SD

---

## COMANDO RÁPIDO (TODO DE GOLPE)

Si usas PlatformIO en terminal:
```bash
cd esp32-bt-mp3-player/firmware
platformio run --target upload
platformio device monitor
```

Eso es todo. ¡Listo para probar!

---

*Guía Rápida v1.0 - Español - Febrero 2025*
