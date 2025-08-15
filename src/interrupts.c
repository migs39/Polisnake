#include "interrupts.h"
#include "timer.h"
#include "uart.h"
#include "kernel.h"

// Interrupt controller registers
#define IRQ_BASIC_PENDING   (INTERRUPT_BASE + 0x200)
#define IRQ_PENDING_1       (INTERRUPT_BASE + 0x204)
#define IRQ_PENDING_2       (INTERRUPT_BASE + 0x208)
#define FIQ_CONTROL         (INTERRUPT_BASE + 0x20C)
#define ENABLE_IRQS_1       (INTERRUPT_BASE + 0x210)
#define ENABLE_IRQS_2       (INTERRUPT_BASE + 0x214)
#define ENABLE_BASIC_IRQS   (INTERRUPT_BASE + 0x218)
#define DISABLE_IRQS_1      (INTERRUPT_BASE + 0x21C)
#define DISABLE_IRQS_2      (INTERRUPT_BASE + 0x220)
#define DISABLE_BASIC_IRQS  (INTERRUPT_BASE + 0x224)

// IRQ numbers
#define IRQ_TIMER_1         1
#define IRQ_UART            57

// Basic IRQ bits
#define BASIC_IRQ_TIMER     (1 << 0)
#define BASIC_IRQ_MAILBOX   (1 << 1)
#define BASIC_IRQ_DOORBELL0 (1 << 2)
#define BASIC_IRQ_DOORBELL1 (1 << 3)
#define BASIC_IRQ_GPU0_HALT (1 << 4)
#define BASIC_IRQ_GPU1_HALT (1 << 5)
#define BASIC_IRQ_ACCESS_ERR_1 (1 << 6)
#define BASIC_IRQ_ACCESS_ERR_0 (1 << 7)

extern void vectors_start(void);

void interrupts_init(void) {
    // Install vector table
    uint32_t vector_table = (uint32_t)&vectors_start;
    
    // Set vector table base address
    __asm__ volatile (
        "mcr p15, 0, %0, c12, c0, 0"
        :
        : "r" (vector_table)
        : "memory"
    );
    
    // Clear all pending interrupts
    mmio_write(IRQ_BASIC_PENDING, 0);
    mmio_write(IRQ_PENDING_1, 0);
    mmio_write(IRQ_PENDING_2, 0);
    
    // Disable all interrupts initially
    mmio_write(DISABLE_BASIC_IRQS, 0xFFFFFFFF);
    mmio_write(DISABLE_IRQS_1, 0xFFFFFFFF);
    mmio_write(DISABLE_IRQS_2, 0xFFFFFFFF);
    
    // Enable timer interrupt (IRQ 1)
    mmio_write(ENABLE_IRQS_1, (1 << IRQ_TIMER_1));
    
    // Enable UART interrupt (IRQ 57)
    mmio_write(ENABLE_IRQS_2, (1 << (IRQ_UART - 32)));
    
    // Set timer for periodic interrupts (every 10ms)
    timer_set_interval(10);
    
    // Enable interrupts in CPU
    enable_interrupts();
}

void handle_irq(void) {
    uint32_t basic_pending = mmio_read(IRQ_BASIC_PENDING);
    uint32_t pending_1 = mmio_read(IRQ_PENDING_1);
    uint32_t pending_2 = mmio_read(IRQ_PENDING_2);
    
    // Handle timer interrupt
    if (pending_1 & (1 << IRQ_TIMER_1)) {
        timer_handle_interrupt();
    }
    
    // Handle UART interrupt  
    if (pending_2 & (1 << (IRQ_UART - 32))) {
        // UART receive interrupt - read available characters
        int c;
        while ((c = uart_getc_nonblocking()) != -1) {
            // Handle input for snake game
            handle_uart_input((char)c);
        }
    }
    
    // Handle basic IRQs
    if (basic_pending) {
        // Handle other basic interrupts if needed
    }
}

// Weak symbol - can be overridden by snake.c
__attribute__((weak)) void handle_uart_input(char c) {
    // Default: echo the character
    uart_putc(c);
}

void interrupts_enable_irq(int irq) {
    if (irq < 32) {
        mmio_write(ENABLE_IRQS_1, (1 << irq));
    } else if (irq < 64) {
        mmio_write(ENABLE_IRQS_2, (1 << (irq - 32)));
    }
}

void interrupts_disable_irq(int irq) {
    if (irq < 32) {
        mmio_write(DISABLE_IRQS_1, (1 << irq));
    } else if (irq < 64) {
        mmio_write(DISABLE_IRQS_2, (1 << (irq - 32)));
    }
}