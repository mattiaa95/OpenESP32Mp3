#include "display_ssd1306.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <cstring>

/* ============================================================================
 * SSD1306 OLED Display Driver
 * 128x64 monochrome display via I2C
 * ========================================================================== */

class DisplaySSD1306Impl : public DisplaySSD1306 {
private:
    /* Use config.h constants */
    static const uint8_t DISP_ADDR = SSD1306_I2C_ADDR;
    
    /* Framebuffer: 128 pixels wide, 8 pages (8 pixels per byte vertically) */
    uint8_t framebuffer[FRAMEBUFFER_SIZE];
    
    /* Dirty tracking: one bit per 8x8 pixel block */
    uint8_t dirty_regions[(DISPLAY_WIDTH / 8) * (DISPLAY_HEIGHT / 8)];
    
    bool initialized = false;
    uint8_t contrast = 0xFF;
    bool powered = true;
    
    void send_command(uint8_t cmd);
    void send_data(const uint8_t* data, size_t len);
    void update_region(uint8_t page, uint8_t col);
    
public:
    bool init() override;
    void clear() override;
    void draw_pixel(uint8_t x, uint8_t y, uint8_t color) override;
    void draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color) override;
    void draw_vline(uint8_t x, uint8_t y, uint8_t height, uint8_t color) override;
    void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) override;
    void draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) override;
    void draw_text(uint8_t x, uint8_t y, const char* text, uint8_t color) override;
    void update() override;
    void update_full() override;
    void set_contrast(uint8_t value) override;
    void sleep() override;
    void wake() override;
};

void DisplaySSD1306Impl::send_command(uint8_t cmd) {
    Wire.beginTransmission(DISP_ADDR);
    Wire.write(0x80);  /* Control byte: command */
    Wire.write(cmd);
    Wire.endTransmission();
}

void DisplaySSD1306Impl::send_data(const uint8_t* data, size_t len) {
    Wire.beginTransmission(DISP_ADDR);
    Wire.write(0x40);  /* Control byte: data stream */
    for (size_t i = 0; i < len; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

bool DisplaySSD1306Impl::init() {
    Serial.println("[OLED] Initializing SSD1306 display");
    
    /* Initialize I2C */
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ * 1000);
    delay(100);
    
    /* Check if device responds */
    Wire.beginTransmission(DISP_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("[OLED] FATAL: SSD1306 not found on I2C bus");
        return false;
    }
    
    /* Hardware initialization sequence */
    send_command(0xAE);  /* Display off */
    send_command(0xD5); send_command(0x80);  /* Clock div ratio */
    send_command(0xA8); send_command(0x3F);  /* Multiplex ratio (1/64) */
    send_command(0xD3); send_command(0x00);  /* Display offset */
    send_command(0x40);  /* Start line */
    send_command(0x8D); send_command(0x14);  /* Charge pump enable */
    send_command(0xA1);  /* Segment re-map */
    send_command(0xC8);  /* COM output direction */
    send_command(0xDA); send_command(0x12);  /* COM pins config */
    send_command(0x81); send_command(0xCF);  /* Contrast */
    send_command(0xD9); send_command(0xF1);  /* Pre-charge period */
    send_command(0xDB); send_command(0x40);  /* V_COMH */
    send_command(0x2E);  /* Deactivate scroll */
    send_command(0xAF);  /* Display on */
    
    delay(100);
    
    /* Initialize framebuffer */
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(dirty_regions, 0xFF, sizeof(dirty_regions));  /* All dirty */
    
    initialized = true;
    Serial.println("[OLED] Initialized successfully");
    return true;
}

void DisplaySSD1306Impl::clear() {
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(dirty_regions, 0xFF, sizeof(dirty_regions));  /* Mark all dirty */
}

void DisplaySSD1306Impl::draw_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return;
    }
    
    uint16_t byte_idx = (y / 8) * DISPLAY_WIDTH + x;
    uint8_t bit = 1 << (y % 8);
    
    if (color) {
        framebuffer[byte_idx] |= bit;
    } else {
        framebuffer[byte_idx] &= ~bit;
    }
    
    /* Mark region dirty */
    uint8_t region_idx = (y / 8) * (DISPLAY_WIDTH / 8) + (x / 8);
    dirty_regions[region_idx] = 1;
}

void DisplaySSD1306Impl::draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color) {
    for (uint8_t i = 0; i < width && (x + i) < DISPLAY_WIDTH; i++) {
        draw_pixel(x + i, y, color);
    }
}

void DisplaySSD1306Impl::draw_vline(uint8_t x, uint8_t y, uint8_t height, uint8_t color) {
    for (uint8_t i = 0; i < height && (y + i) < DISPLAY_HEIGHT; i++) {
        draw_pixel(x, y + i, color);
    }
}

void DisplaySSD1306Impl::draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    draw_hline(x, y, w, color);
    draw_hline(x, y + h - 1, w, color);
    draw_vline(x, y, h, color);
    draw_vline(x + w - 1, y, h, color);
}

void DisplaySSD1306Impl::draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t i = 0; i < h; i++) {
        draw_hline(x, y + i, w, color);
    }
}

void DisplaySSD1306Impl::draw_text(uint8_t x, uint8_t y, const char* text, uint8_t color) {
    if (!text) return;
    
    /* Simple monospace text: 6 pixels wide per character */
    uint8_t char_width = 6;
    for (size_t i = 0; text[i] && (x + i * char_width < DISPLAY_WIDTH); i++) {
        /* Placeholder: just draw boxes for now */
        draw_filled_rect(x + i * char_width, y, 5, 7, color);
    }
}

void DisplaySSD1306Impl::update() {
    /* Partial update using dirty regions */
    for (uint8_t page = 0; page < 8; page++) {
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t region_idx = page * 16 + col;
            if (dirty_regions[region_idx]) {
                update_region(page, col);
                dirty_regions[region_idx] = 0;
            }
        }
    }
}

void DisplaySSD1306Impl::update_full() {
    /* Full display update: 8 pages, each 128 bytes */
    for (uint8_t page = 0; page < 8; page++) {
        /* Set page and column */
        send_command(0xB0 | page);
        send_command(0x00);
        send_command(0x10);
        
        /* Send 128 bytes of data for this page */
        send_data(&framebuffer[page * DISPLAY_WIDTH], DISPLAY_WIDTH);
    }
}

void DisplaySSD1306Impl::update_region(uint8_t page, uint8_t col) {
    if (page >= 8 || col >= 16) return;
    
    /* Set page and column (8 bytes per region) */
    send_command(0xB0 | page);
    send_command(0x00 | (col * 8));
    send_command(0x10);
    
    /* Send 8 bytes for this region */
    uint16_t offset = page * DISPLAY_WIDTH + col * 8;
    send_data(&framebuffer[offset], 8);
}

void DisplaySSD1306Impl::set_contrast(uint8_t value) {
    contrast = value;
    send_command(0x81);
    send_command(value);
}

void DisplaySSD1306Impl::sleep() {
    powered = false;
    send_command(0xAE);  /* Display off */
}

void DisplaySSD1306Impl::wake() {
    powered = true;
    send_command(0xAF);  /* Display on */
}

/* Global singleton */
static DisplaySSD1306Impl g_display;

DisplaySSD1306* create_display_ssd1306() {
    return &g_display;
}
