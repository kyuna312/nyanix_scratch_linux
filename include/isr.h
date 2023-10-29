#ifndef __ISR_H 
#define __ISR_H 

#include <type.h>

/* Reference: http://wiki.osdev.org/Interrupt_Service_Routines */

// IDT.h only sets the interrupt vector, but here is the interrupt service routine ISR, which handles interrupts.
//Interrupt handler and interrupt service routine are different
// All interrupts are handled by the interrupt handler and then dispatched to each interrupt service routine

#define ISR_IRQ0    32 // Handle interrupts from 32-47
#define ISR_NIRQ    16
#define ISR_SYSCALL 0x80 // System call interrupt, it is not used yet
#define ISR_UNKNOWN 255

// Register data to be saved during interruption
struct interrupt_frame {
    /* segment registers */
    uint32_t gs;    // 16 bits
    uint32_t fs;    // 16 bits
    uint32_t es;    // 16 bits
    uint32_t ds;    // 16 bits

    /* registers save by pusha */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ret_addr; // return address
    uint32_t int_no;

    /* save by `int` instruction */
    uint32_t eip;
    uint32_t cs;    // 16 bits
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t ss;    // 16 bits
};    

// Initialize interrupt service routine
void isr_init();

#endif
