#ifndef DISPLAY_SSD1306_H
#define DISPLAY_SSD1306_H

#include <cstdint>
#include <cstddef>

/* ============================================================================
 * SSD1306 OLED Display Interface (Pure Virtual)
 * I2C-based display driver with framebuffer management
 * ========================================================================== */

class DisplaySSD1306 {
public:
    virtual ~DisplaySSD1306() = default;
    
    /* Initialize I2C and display hardware */
    virtual bool init() = 0;
    
    /* Clear display (all pixels off) */
    virtual void clear() = 0;
    
    /* Draw pixel at (x, y); color: 0 = off, 1 = on */
    virtual void draw_pixel(uint8_t x, uint8_t y, uint8_t color) = 0;
    
    /* Draw horizontal line */
    virtual void draw_hline(uint8_t x, uint8_t y, uint8_t width, uint8_t color) = 0;
    
    /* Draw vertical line */
    virtual void draw_vline(uint8_t x, uint8_t y, uint8_t height, uint8_t color) = 0;
    
    /* Draw rectangle (outline) */
    virtual void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) = 0;
    
    /* Draw filled rectangle */
    virtual void draw_filled_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) = 0;
    
    /* Draw text (monospace font, 5x7 pixels per character) */
    virtual void draw_text(uint8_t x, uint8_t y, const char* text, uint8_t color) = 0;
    
    /* Display dirty-tracked partial update (optimized) */
    virtual void update() = 0;
    
    /* Full display update (slower; use sparingly) */
    virtual void update_full() = 0;
    
    /* Set contrast (0–255) */
    virtual void set_contrast(uint8_t value) = 0;
    
    /* Power down display */
    virtual void sleep() = 0;
    
    /* Wake up display */
    virtual void wake() = 0;
};

#endif  // DISPLAY_SSD1306_H
