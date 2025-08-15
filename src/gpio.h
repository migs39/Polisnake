#ifndef GPIO_H
#define GPIO_H

#include "kernel.h"

// GPIO function select
typedef enum {
    GPIO_FUNC_INPUT = 0,
    GPIO_FUNC_OUTPUT = 1,
    GPIO_FUNC_ALT0 = 4,
    GPIO_FUNC_ALT1 = 5,
    GPIO_FUNC_ALT2 = 6,
    GPIO_FUNC_ALT3 = 7,
    GPIO_FUNC_ALT4 = 3,
    GPIO_FUNC_ALT5 = 2
} gpio_function_t;

// GPIO pull up/down
typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_DOWN = 1,
    GPIO_PULL_UP = 2
} gpio_pull_t;

// Function declarations
void gpio_init(void);
void gpio_set_function(int pin, gpio_function_t func);
void gpio_set_output(int pin, int value);
int gpio_get_input(int pin);
void gpio_set_pull(int pin, gpio_pull_t pull);

// Button functions (using specific pins)
int gpio_read_button_up(void);    // GPIO 2
int gpio_read_button_down(void);  // GPIO 3  
int gpio_read_button_left(void);  // GPIO 4
int gpio_read_button_right(void); // GPIO 17

// LED functions
void gpio_led_on(void);   // GPIO 18
void gpio_led_off(void);  // GPIO 18

#endif