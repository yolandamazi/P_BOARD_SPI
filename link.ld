/*
 * The stack starts at the top of memory, defined by __sp, and grows down.
 * Unfortunately this means we must hardcode the size of memory.
 * Here we choose 64K. Change this value to change the working memory size.
 */
 
 /* NOEL RAM starts at ZERO */
 
__mem   = 0x00000000; /*This should be set to the beginning of the memory as specified by the device tree, for spike run: "spike --dump-dts a.out"*/

__stack = 0x00100000; /*This is the offset that the stack is from the start of memory.*/

SECTIONS
{
    . = __mem;
    .text : {
        *(.text.init) /* add the instruction from init.s*/
        *(.text) /* add the instructions compiled */
    }

    .data : {
        *(COMMON)
        *(.sdata)
        *(.rodata)
        *(.data*)
        *(.got*)
    }

    .bss : {
        *(.sbss) 
        *(.bss) 
    }
    
    .commnent : { *(.comment) }
    .riscv.attributes : { *(.riscv.attributes) }
}
