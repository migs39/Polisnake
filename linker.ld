/* Linker script for Raspberry Pi 2 kernel */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 0x8000 on RPi2 */
    . = 0x8000;
    
    .text : {
        KEEP(*(.text.boot))
        *(.text)
    }
    
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
    
    /* Stack grows downward from 0x8000 */
    __stack_start = 0x8000;
    
    /* Heap starts after BSS */
    __heap_start = .;
    
    /DISCARD/ : {
        *(.comment)
        *(.gnu*)
        *(.note*)
        *(.eh_frame*)
    }
}