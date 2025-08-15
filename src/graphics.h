#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kernel.h"
#include "framebuffer.h"

// Function declarations
void graphics_clear_screen(uint32_t color);
void graphics_draw_pixel(int x, int y, uint32_t color);
void graphics_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint32_t color);
void graphics_draw_rect_outline(int x, int y, int width, int height, uint32_t color);
void graphics_draw_circle(int cx, int cy, int radius, uint32_t color);
void graphics_draw_char(char c, int x, int y, uint32_t color);
void graphics_draw_text(const char* text, int x, int y, uint32_t color);

// Helper function
int abs(int x);

#endif