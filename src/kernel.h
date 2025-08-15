#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Raspberry Pi 2 specific defines
#define PERIPHERAL_BASE     0x3F000000
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000)
#define UART_BASE           (PERIPHERAL_BASE + 0x201000)
#define TIMER_BASE          (PERIPHERAL_BASE + 0x003000)
#define INTERRUPT_BASE      (PERIPHERAL_BASE + 0x00B000)
#define MAILBOX_BASE        (PERIPHERAL_BASE + 0x00B880)

// Memory management
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __heap_start;
extern uint32_t __stack_start;

// Basic types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// Utility macros
#define NULL ((void*)0)
#define TRUE 1
#define FALSE 0

// Memory operations
static inline void memory_barrier(void) {
    __asm__ volatile ("dmb" ::: "memory");
}

static inline void dsb(void) {
    __asm__ volatile ("dsb" ::: "memory");
}

static inline void isb(void) {
    __asm__ volatile ("isb" ::: "memory");
}

// Register access
static inline void mmio_write(uint32_t reg, uint32_t data) {
    memory_barrier();
    *(volatile uint32_t*)reg = data;
    memory_barrier();
}

static inline uint32_t mmio_read(uint32_t reg) {
    memory_barrier();
    return *(volatile uint32_t*)reg;
    memory_barrier();
}

// Delay function
static inline void delay(int32_t count) {
    __asm__ volatile("1: subs %0, %0, #1; bne 1b" : "=r"(count) : "0"(count) : "cc");
}

// Function declarations
void kernel_main(void);
void panic(const char* message);

// Assembly functions
extern void enable_interrupts(void);
extern void disable_interrupts(void);

#endif