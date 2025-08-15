#include "uart.h"
#include "gpio.h"
#include "kernel.h"

// UART registers
#define UART_DR         (UART_BASE + 0x00)
#define UART_RSRECR     (UART_BASE + 0x04)
#define UART_FR         (UART_BASE + 0x18)
#define UART_ILPR       (UART_BASE + 0x20)
#define UART_IBRD       (UART_BASE + 0x24)
#define UART_FBRD       (UART_BASE + 0x28)
#define UART_LCRH       (UART_BASE + 0x2C)
#define UART_CR         (UART_BASE + 0x30)
#define UART_IFLS       (UART_BASE + 0x34)
#define UART_IMSC       (UART_BASE + 0x38)
#define UART_RIS        (UART_BASE + 0x3C)
#define UART_MIS        (UART_BASE + 0x40)
#define UART_ICR        (UART_BASE + 0x44)
#define UART_DMACR      (UART_BASE + 0x48)
#define UART_ITCR       (UART_BASE + 0x80)
#define UART_ITIP       (UART_BASE + 0x84)
#define UART_ITOP       (UART_BASE + 0x88)
#define UART_TDR        (UART_BASE + 0x8C)

// UART_FR bits
#define UART_FR_RXFE    0x10    // Receive FIFO empty
#define UART_FR_TXFF    0x20    // Transmit FIFO full
#define UART_FR_RXFF    0x40    // Receive FIFO full
#define UART_FR_TXFE    0x80    // Transmit FIFO empty

// UART_LCRH bits
#define UART_LCRH_FEN   0x10    // Enable FIFOs
#define UART_LCRH_WLEN8 0x60    // 8-bit word length

// UART_CR bits
#define UART_CR_UARTEN  0x01    // UART enable
#define UART_CR_TXE     0x100   // Transmit enable
#define UART_CR_RXE     0x200   // Receive enable

// UART_IMSC bits
#define UART_IMSC_RXIM  0x10    // Receive interrupt mask

static volatile int uart_initialized = 0;

void uart_init(void) {
    // Disable UART
    mmio_write(UART_CR, 0);
    
    // Setup GPIO pins 14 & 15 for UART
    gpio_set_function(14, GPIO_FUNC_ALT0); // TX
    gpio_set_function(15, GPIO_FUNC_ALT0); // RX
    
    // Disable pull-up/down for pins 14 & 15
    gpio_set_pull(14, GPIO_PULL_NONE);
    gpio_set_pull(15, GPIO_PULL_NONE);
    
    // Clear pending interrupts
    mmio_write(UART_ICR, 0x7FF);
    
    // Set baud rate: 115200
    // UART clock = 48MHz on RPi2
    // Divisor = 48000000 / (16 * 115200) = 26.041666...
    // Integer part: 26
    // Fractional part: 0.041666... * 64 = 2.666... ≈ 3
    mmio_write(UART_IBRD, 26);
    mmio_write(UART_FBRD, 3);
    
    // Set 8N1 (8 bits, no parity, 1 stop bit) and enable FIFO
    mmio_write(UART_LCRH, UART_LCRH_WLEN8 | UART_LCRH_FEN);
    
    // Enable receive interrupts
    mmio_write(UART_IMSC, UART_IMSC_RXIM);
    
    // Enable UART, TX and RX
    mmio_write(UART_CR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
    
    uart_initialized = 1;
}

void uart_putc(char c) {
    if (!uart_initialized) return;
    
    // Wait for transmit FIFO to not be full
    while (mmio_read(UART_FR) & UART_FR_TXFF) {
        // Wait
    }
    
    // Write character
    mmio_write(UART_DR, c);
}

char uart_getc(void) {
    if (!uart_initialized) return 0;
    
    // Wait for receive FIFO to not be empty
    while (mmio_read(UART_FR) & UART_FR_RXFE) {
        // Wait
    }
    
    // Read character
    return mmio_read(UART_DR) & 0xFF;
}

int uart_getc_nonblocking(void) {
    if (!uart_initialized) return -1;
    
    // Check if receive FIFO is empty
    if (mmio_read(UART_FR) & UART_FR_RXFE) {
        return -1; // No character available
    }
    
    // Read character
    return mmio_read(UART_DR) & 0xFF;
}

void uart_puts(const char* str) {
    if (!uart_initialized) return;
    
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r'); // Convert LF to CRLF
        }
        uart_putc(*str++);
    }
}

void uart_hex(uint32_t value) {
    if (!uart_initialized) return;
    
    uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint32_t digit = (value >> (i * 4)) & 0xF;
        if (digit < 10) {
            uart_putc('0' + digit);
        } else {
            uart_putc('A' + digit - 10);
        }
    }
}

void uart_dec(uint32_t value) {
    if (!uart_initialized) return;
    
    if (value == 0) {
        uart_putc('0');
        return;
    }
    
    char buffer[32];
    int pos = 0;
    
    while (value > 0) {
        buffer[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Print digits in reverse order
    for (int i = pos - 1; i >= 0; i--) {
        uart_putc(buffer[i]);
    }
}