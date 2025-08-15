#ifndef UART_H
#define UART_H

#include "kernel.h"

// Function declarations
void uart_init(void);
void uart_putc(char c);
char uart_getc(void);
int uart_getc_nonblocking(void);
void uart_puts(const char* str);
void uart_hex(uint32_t value);
void uart_dec(uint32_t value);

#endif