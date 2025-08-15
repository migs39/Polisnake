#include "gpio.h"
#include "kernel.h"

// GPIO registers
#define GPFSEL0     (GPIO_BASE + 0x00)
#define GPFSEL1     (GPIO_BASE + 0x04)
#define GPFSEL2     (GPIO_BASE + 0x08)
#define GPFSEL3     (GPIO_BASE + 0x0C)
#define GPFSEL4     (GPIO_BASE + 0x10)
#define GPFSEL5     (GPIO_BASE + 0x14)

#define GPSET0      (GPIO_BASE + 0x1C)
#define GPSET1      (GPIO_BASE + 0x20)

#define GPCLR0      (GPIO_BASE + 0x28)
#define GPCLR1      (GPIO_BASE + 0x2C)

#define GPLEV0      (GPIO_BASE + 0x34)
#define GPLEV1      (GPIO_BASE + 0x38)

#define GPEDS0      (GPIO_BASE + 0x40)
#define GPEDS1      (GPIO_BASE + 0x44)

#define GPREN0      (GPIO_BASE + 0x4C)
#define GPREN1      (GPIO_BASE + 0x50)

#define GPFEN0      (GPIO_BASE + 0x58)
#define GPFEN1      (GPIO_BASE + 0x5C)

#define GPHEN0      (GPIO_BASE + 0x64)
#define GPHEN1      (GPIO_BASE + 0x68)

#define GPLEN0      (GPIO_BASE + 0x70)
#define GPLEN1      (GPIO_BASE + 0x74)

#define GPAREN0     (GPIO_BASE + 0x7C)
#define GPAREN1     (GPIO_BASE + 0x80)

#define GPAFEN0     (GPIO_BASE + 0x88)
#define GPAFEN1     (GPIO_BASE + 0x8C)

#define GPPUD       (GPIO_BASE + 0x94)
#define GPPUDCLK0   (GPIO_BASE + 0x98)
#define GPPUDCLK1   (GPIO_BASE + 0x9C)

void gpio_init(void) {
    // Initialize some pins for buttons (optional)
    // GPIO 2, 3, 4, 17 as inputs with pull-up for buttons
    gpio_set_function(2, GPIO_FUNC_INPUT);
    gpio_set_function(3, GPIO_FUNC_INPUT);
    gpio_set_function(4, GPIO_FUNC_INPUT);
    gpio_set_function(17, GPIO_FUNC_INPUT);
    
    gpio_set_pull(2, GPIO_PULL_UP);
    gpio_set_pull(3, GPIO_PULL_UP);
    gpio_set_pull(4, GPIO_PULL_UP);
    gpio_set_pull(17, GPIO_PULL_UP);
    
    // GPIO 18 as output for status LED
    gpio_set_function(18, GPIO_FUNC_OUTPUT);
}

void gpio_set_function(int pin, gpio_function_t func) {
    if (pin < 0 || pin > 53) return;
    
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    uint32_t sel_reg = GPIO_BASE + (reg * 4);
    
    uint32_t value = mmio_read(sel_reg);
    value &= ~(7 << shift);  // Clear the 3 bits
    value |= (func << shift); // Set new function
    mmio_write(sel_reg, value);
}

void gpio_set_output(int pin, int value) {
    if (pin < 0 || pin > 53) return;
    
    if (value) {
        if (pin < 32) {
            mmio_write(GPSET0, 1 << pin);
        } else {
            mmio_write(GPSET1, 1 << (pin - 32));
        }
    } else {
        if (pin < 32) {
            mmio_write(GPCLR0, 1 << pin);
        } else {
            mmio_write(GPCLR1, 1 << (pin - 32));
        }
    }
}

int gpio_get_input(int pin) {
    if (pin < 0 || pin > 53) return 0;
    
    uint32_t reg;
    if (pin < 32) {
        reg = mmio_read(GPLEV0);
        return (reg >> pin) & 1;
    } else {
        reg = mmio_read(GPLEV1);
        return (reg >> (pin - 32)) & 1;
    }
}

void gpio_set_pull(int pin, gpio_pull_t pull) {
    if (pin < 0 || pin > 53) return;
    
    // Set pull-up/down control
    mmio_write(GPPUD, pull);
    
    // Wait 150 cycles
    delay(150);
    
    // Clock the control signal into the pin
    if (pin < 32) {
        mmio_write(GPPUDCLK0, 1 << pin);
    } else {
        mmio_write(GPPUDCLK1, 1 << (pin - 32));
    }
    
    // Wait 150 cycles
    delay(150);
    
    // Remove control signal and clock
    mmio_write(GPPUD, 0);
    mmio_write(GPPUDCLK0, 0);
    mmio_write(GPPUDCLK1, 0);
}

// Simple button reading functions
int gpio_read_button_up(void) {
    return !gpio_get_input(2);  // Active low
}

int gpio_read_button_down(void) {
    return !gpio_get_input(3);  // Active low
}

int gpio_read_button_left(void) {
    return !gpio_get_input(4);  // Active low
}

int gpio_read_button_right(void) {
    return !gpio_get_input(17); // Active low
}

void gpio_led_on(void) {
    gpio_set_output(18, 1);
}

void gpio_led_off(void) {
    gpio_set_output(18, 0);
}