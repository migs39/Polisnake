.section .text

.global vectors_start
.global enable_interrupts

// Exception vector table
vectors_start:
    ldr pc, =reset_handler
    ldr pc, =undefined_handler
    ldr pc, =swi_handler
    ldr pc, =prefetch_handler
    ldr pc, =data_handler
    ldr pc, =unused_handler
    ldr pc, =irq_handler
    ldr pc, =fiq_handler

// Exception handlers
reset_handler:
    b _start

undefined_handler:
    b halt

swi_handler:
    b halt

prefetch_handler:
    b halt

data_handler:
    b halt

unused_handler:
    b halt

// IRQ Handler - saves context and calls C handler
irq_handler:
    // Save context on IRQ stack
    push {r0-r12, lr}
    
    // Call C IRQ handler
    bl handle_irq
    
    // Restore context
    pop {r0-r12, lr}
    
    // Return from IRQ
    subs pc, lr, #4

fiq_handler:
    b halt

// Enable interrupts
enable_interrupts:
    cpsie if
    bx lr

// Disable interrupts  
.global disable_interrupts
disable_interrupts:
    cpsid if
    bx lr

halt:
    wfi
    b halt
