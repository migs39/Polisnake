# Raspberry Pi 2 Snake Baremetal Project
# Cross-compilation setup for ARMv7-A

# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

# Compiler flags
CFLAGS = -mcpu=cortex-a7 -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra
CFLAGS += -nostdlib -nostartfiles -nodefaultlibs
ASFLAGS = -mcpu=cortex-a7

# Directories
SRC_DIR = src
BUILD_DIR = build
BOOT_DIR = boot

# Source files
ASM_SOURCES = $(BOOT_DIR)/boot.s $(SRC_DIR)/vectors.s
C_SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/framebuffer.c $(SRC_DIR)/gpio.c \
           $(SRC_DIR)/timer.c $(SRC_DIR)/uart.c $(SRC_DIR)/interrupts.c \
           $(SRC_DIR)/snake.c $(SRC_DIR)/graphics.c

# Object files
ASM_OBJECTS = $(ASM_SOURCES:%.s=$(BUILD_DIR)/%.o)
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Target
TARGET = kernel.img

.PHONY: all clean run install

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/$(BOOT_DIR) $(BUILD_DIR)/$(SRC_DIR)

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

kernel.elf: $(OBJECTS)
	$(LD) -T linker.ld $(OBJECTS) -o $@

$(TARGET): kernel.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR) kernel.elf $(TARGET)

# Run on QEMU (for testing)
run: $(TARGET)
	qemu-system-arm -M raspi2b -kernel $(TARGET) -serial stdio

# Install to SD card (change /dev/sdX to your SD card)
install: $(TARGET)
	@echo "Make sure to change /dev/sdX to your actual SD card device!"
	@echo "sudo cp $(TARGET) /mnt/sdcard/"
	@echo "sudo sync"

# Debug version
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all     - Build kernel.img"
	@echo "  clean   - Clean build files"
	@echo "  run     - Run on QEMU"
	@echo "  install - Install to SD card (manual step)"
	@echo "  debug   - Build debug version"