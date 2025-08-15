#ifndef TIMER_H
#define TIMER_H

#include "kernel.h"

// Function declarations
void timer_init(void);
uint32_t timer_get_ticks(void);
uint64_t timer_get_ticks_64(void);
void timer_sleep(uint32_t milliseconds);
void timer_sleep_us(uint32_t microseconds);
uint32_t timer_get_system_timer(void);
void timer_set_interval(uint32_t interval_ms);
void timer_handle_interrupt(void);

#endif