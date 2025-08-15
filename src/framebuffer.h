#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "kernel.h"

// Screen configuration
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define SCREEN_DEPTH    32

// Color definitions (ARGB format)
#define COLOR_BLACK     0xFF000000
#define COLOR_WHITE     0xFFFFFFFF
#define COLOR_RED       0xFFFF0000
#define COLOR_GREEN     0xFF00FF00
#define COLOR_BLUE      0xFF0000FF
#define COLOR_YELLOW    0xFFFFFF00
#define COLOR_CYAN      0xFF00FFFF
#define COLOR_MAGENTA   0xFFFF00FF
#define COLOR_GRAY      0xFF808080
#define COLOR_DARK_GRAY 0xFF404040

// Framebuffer structure
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t* buffer;
    uint32_t size;
} framebuffer_t;

// Function declarations
int framebuffer_init(void);
framebuffer_t* framebuffer_get(void);
void framebuffer_put_pixel(int x, int y, uint32_t color);
uint32_t framebuffer_get_pixel(int x, int y);
void framebuffer_clear(uint32_t color);

// Inline color utilities
static inline uint32_t make_color(uint8_t r, uint8_t g, uint8_t b) {
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

static inline uint8_t get_red(uint32_t color) {
    return (color >> 16) & 0xFF;
}

static inline uint8_t get_green(uint32_t color) {
    return (color >> 8) & 0xFF;
}

static inline uint8_t get_blue(uint32_t color) {
    return color & 0xFF;
}

#endif