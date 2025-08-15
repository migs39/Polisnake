#include "timer.h"
#include "kernel.h"

// System Timer registers
#define TIMER_CS    (TIMER_BASE + 0x00)
#define TIMER_CLO   (TIMER_BASE + 0x04)
#define TIMER_CHI   (TIMER_BASE + 0x08)
#define TIMER_C0    (TIMER_BASE + 0x0C)
#define TIMER_C1    (TIMER_BASE + 0x10)
#define TIMER_C2    (TIMER_BASE + 0x14)
#define TIMER_C3    (TIMER_BASE + 0x18)

// Timer frequency is 1MHz
#define TIMER_FREQ  1000000

static volatile uint32_t system_timer_tick = 0;

void timer_init(void) {
    // Clear any pending timer interrupts
    mmio_write(TIMER_CS, 0xF);
    
    // Reset our tick counter
    system_timer_tick = 0;
}

uint32_t timer_get_ticks(void) {
    return mmio_read(TIMER_CLO);
}

uint64_t timer_get_ticks_64(void) {
    uint32_t hi = mmio_read(TIMER_CHI);
    uint32_t lo = mmio_read(TIMER_CLO);
    
    // Check if low counter wrapped during read
    if (mmio_read(TIMER_CHI) != hi) {
        hi = mmio_read(TIMER_CHI);
        lo = mmio_read(TIMER_CLO);
    }
    
    return ((uint64_t)hi << 32) | lo;
}

void timer_sleep(uint32_t milliseconds) {
    uint32_t start = timer_get_ticks();
    uint32_t target = start + (milliseconds * 1000); // Convert ms to microseconds
    
    // Handle wrap-around
    if (target < start) {
        // Wait for wrap
        while (timer_get_ticks() >= start) {
            __asm__("nop");
        }
    }
    
    // Wait for target
    while (timer_get_ticks() < target) {
        __asm__("nop");
    }
}

void timer_sleep_us(uint32_t microseconds) {
    uint32_t start = timer_get_ticks();
    uint32_t target = start + microseconds;
    
    // Handle wrap-around
    if (target < start) {
        // Wait for wrap
        while (timer_get_ticks() >= start) {
            __asm__("nop");
        }
    }
    
    // Wait for target
    while (timer_get_ticks() < target) {
        __asm__("nop");
    }
}

uint32_t timer_get_system_timer(void) {
    return system_timer_tick;
}

void timer_set_interval(uint32_t interval_ms) {
    uint32_t current = timer_get_ticks();
    uint32_t compare = current + (interval_ms * 1000);
    
    // Set timer compare register 1 for periodic interrupt
    mmio_write(TIMER_C1, compare);
}

// Called from interrupt handler
void timer_handle_interrupt(void) {
    // Clear the interrupt
    mmio_write(TIMER_CS, 0x2); // Clear timer 1 match
    
    // Increment our tick counter
    system_timer_tick++;
    
    // Set next interrupt (every 10ms = 10000 microseconds)
    uint32_t current = timer_get_ticks();
    mmio_write(TIMER_C1, current + 10000);
}