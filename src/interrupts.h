#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "kernel.h"

// Function declarations
void interrupts_init(void);
void handle_irq(void);
void handle_uart_input(char c);
void interrupts_enable_irq(int irq);
void interrupts_disable_irq(int irq);

#endif