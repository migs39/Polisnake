#include "framebuffer.h"
#include "kernel.h"

// Mailbox channels
#define MAILBOX_CHANNEL_PROP    8

// Mailbox registers
#define MAILBOX_READ    (MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS  (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (MAILBOX_BASE + 0x20)

// Mailbox status bits
#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

// Property tags
#define TAG_SET_PHYSICAL_SIZE   0x48003
#define TAG_SET_VIRTUAL_SIZE    0x48004
#define TAG_SET_DEPTH           0x48005
#define TAG_ALLOCATE_BUFFER     0x40001
#define TAG_GET_PITCH           0x40008

static framebuffer_t fb;

// Mailbox property structure (must be 16-byte aligned)
typedef struct {
    uint32_t size;
    uint32_t code;
    
    // Set physical size
    uint32_t tag_physical;
    uint32_t tag_physical_size;
    uint32_t tag_physical_code;
    uint32_t physical_width;
    uint32_t physical_height;
    
    // Set virtual size  
    uint32_t tag_virtual;
    uint32_t tag_virtual_size;
    uint32_t tag_virtual_code;
    uint32_t virtual_width;
    uint32_t virtual_height;
    
    // Set depth
    uint32_t tag_depth;
    uint32_t tag_depth_size;
    uint32_t tag_depth_code;
    uint32_t depth;
    
    // Allocate buffer
    uint32_t tag_allocate;
    uint32_t tag_allocate_size;
    uint32_t tag_allocate_code;
    uint32_t buffer_addr;
    uint32_t buffer_size;
    
    // Get pitch
    uint32_t tag_pitch;
    uint32_t tag_pitch_size;
    uint32_t tag_pitch_code;
    uint32_t pitch;
    
    uint32_t end_tag;
} __attribute__((aligned(16))) mailbox_property_t;

static mailbox_property_t property_buffer = {
    .size = sizeof(mailbox_property_t),
    .code = 0,
    
    .tag_physical = TAG_SET_PHYSICAL_SIZE,
    .tag_physical_size = 8,
    .tag_physical_code = 0,
    .physical_width = SCREEN_WIDTH,
    .physical_height = SCREEN_HEIGHT,
    
    .tag_virtual = TAG_SET_VIRTUAL_SIZE,
    .tag_virtual_size = 8,
    .tag_virtual_code = 0,
    .virtual_width = SCREEN_WIDTH,
    .virtual_height = SCREEN_HEIGHT,
    
    .tag_depth = TAG_SET_DEPTH,
    .tag_depth_size = 4,
    .tag_depth_code = 0,
    .depth = SCREEN_DEPTH,
    
    .tag_allocate = TAG_ALLOCATE_BUFFER,
    .tag_allocate_size = 8,
    .tag_allocate_code = 0,
    .buffer_addr = 0,
    .buffer_size = 0,
    
    .tag_pitch = TAG_GET_PITCH,
    .tag_pitch_size = 4,
    .tag_pitch_code = 0,
    .pitch = 0,
    
    .end_tag = 0
};

static void mailbox_write(uint8_t channel, uint32_t data) {
    // Wait for mailbox to not be full
    while (mmio_read(MAILBOX_STATUS) & MAILBOX_FULL) {
        // Wait
    }
    
    // Write data with channel
    mmio_write(MAILBOX_WRITE, (data & 0xFFFFFFF0) | (channel & 0xF));
}

static uint32_t mailbox_read(uint8_t channel) {
    uint32_t data;
    
    while (1) {
        // Wait for mailbox to not be empty
        while (mmio_read(MAILBOX_STATUS) & MAILBOX_EMPTY) {
            // Wait
        }
        
        // Read data
        data = mmio_read(MAILBOX_READ);
        
        // Check if it's for our channel
        if ((data & 0xF) == channel) {
            return data & 0xFFFFFFF0;
        }
    }
}

int framebuffer_init(void) {
    // Get physical address of property buffer
    uint32_t addr = (uint32_t)&property_buffer;
    
    // Convert to bus address (add 0xC0000000 for uncached access)
    addr += 0x40000000;
    
    // Send property buffer to GPU
    mailbox_write(MAILBOX_CHANNEL_PROP, addr);
    
    // Read response
    uint32_t result = mailbox_read(MAILBOX_CHANNEL_PROP);
    
    if (result != addr || property_buffer.code != 0x80000000) {
        return -1; // Failed
    }
    
    // Check if buffer allocation succeeded
    if (property_buffer.buffer_addr == 0) {
        return -1;
    }
    
    // Set up framebuffer structure
    fb.width = property_buffer.physical_width;
    fb.height = property_buffer.physical_height;
    fb.pitch = property_buffer.pitch;
    fb.buffer = (uint32_t*)(property_buffer.buffer_addr & 0x3FFFFFFF); // Convert from bus to ARM address
    fb.size = property_buffer.buffer_size;
    
    return 0; // Success
}

framebuffer_t* framebuffer_get(void) {
    return &fb;
}

void framebuffer_put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < fb.width && y >= 0 && y < fb.height) {
        uint32_t* pixel = fb.buffer + (y * fb.width + x);
        *pixel = color;
    }
}

uint32_t framebuffer_get_pixel(int x, int y) {
    if (x >= 0 && x < fb.width && y >= 0 && y < fb.height) {
        uint32_t* pixel = fb.buffer + (y * fb.width + x);
        return *pixel;
    }
    return 0;
}

void framebuffer_clear(uint32_t color) {
    for (int i = 0; i < fb.width * fb.height; i++) {
        fb.buffer[i] = color;
    }
}