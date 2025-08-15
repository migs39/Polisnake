#include "kernel.h"
#include "framebuffer.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "interrupts.h"
#include "snake.h"
#include "graphics.h"

void kernel_main(void) {
    // Initialize UART for debugging
    uart_init();
    uart_puts("RPi2 Snake OS Starting...\n");

    // Initialize framebuffer
    if (framebuffer_init() != 0) {
        uart_puts("Failed to initialize framebuffer\n");
        while (1);
    }
    uart_puts("Framebuffer initialized\n");

    // Initialize GPIO
    gpio_init();
    uart_puts("GPIO initialized\n");

    // Initialize timer
    timer_init();
    uart_puts("Timer initialized\n");

    // Initialize interrupt system
    interrupts_init();
    uart_puts("Interrupts initialized\n");

    // Clear screen
    graphics_clear_screen(COLOR_BLACK);
    
    // Draw title
    graphics_draw_text("RPi2 SNAKE", 10, 10, COLOR_WHITE);
    graphics_draw_text("Use GPIO buttons or UART", 10, 30, COLOR_YELLOW);
    graphics_draw_text("W=Up A=Left S=Down D=Right", 10, 50, COLOR_YELLOW);
    
    // Wait a bit
    timer_sleep(2000);
    
    // Initialize and start snake game
    snake_init();
    uart_puts("Snake game initialized\n");
    uart_puts("Game starting! Use WASD keys via UART\n");
    
    // Main game loop
    while (1) {
        snake_update();
        snake_draw();
        
        // Game runs at ~10 FPS
        timer_sleep(100);
    }
}

// Simple panic function
void panic(const char* message) {
    uart_puts("KERNEL PANIC: ");
    uart_puts(message);
    uart_puts("\n");
    
    // Try to display on screen too
    graphics_clear_screen(COLOR_RED);
    graphics_draw_text("KERNEL PANIC", 10, 10, COLOR_WHITE);
    graphics_draw_text(message, 10, 30, COLOR_WHITE);
    
    // Halt
    while (1) {
        __asm__("wfi");
    }
}