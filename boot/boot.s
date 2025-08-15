.section .text.boot

.global _start

_start:
    // Check if we're running on core 0
    mrc p15, 0, r0, c0, c0, 5   // Read Multiprocessor Affinity Register
    and r0, r0, #3              // Extract CPU ID
    cmp r0, #0                  // Compare with 0
    bne halt                    // If not core 0, halt this core

    // Disable interrupts
    cpsid if

    // Set up stack pointer before BSS (grows downward)
    ldr sp, =__stack_start

    // Clear BSS section
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
clear_bss:
    cmp r0, r1
    bge clear_bss_done
    str r2, [r0], #4
    b clear_bss
clear_bss_done:

    // Disable caches and MMU
    mrc p15, 0, r0, c1, c0, 0   // Read System Control Register
    bic r0, r0, #0x1            // Disable MMU
    bic r0, r0, #0x4            // Disable data cache
    bic r0, r0, #0x1000         // Disable instruction cache
    mcr p15, 0, r0, c1, c0, 0   // Write System Control Register

    // Enable VFP/NEON if available
    mrc p15, 0, r0, c1, c0, 2   // Read Coprocessor Access Control Register
    orr r0, r0, #0x300000       // Enable CP10 and CP11
    orr r0, r0, #0xC00000       // Enable CP10 and CP11 in secure mode
    mcr p15, 0, r0, c1, c0, 2   // Write Coprocessor Access Control Register

    // Enable interrupts in CPSR
    cpsie if

    // Jump to kernel main
    bl kernel_main

halt:
    wfi                         // Wait for interrupt (low power)
    b halt                      // Loop forever