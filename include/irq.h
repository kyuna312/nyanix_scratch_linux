#ifndef __IRQ_H 
#define __IRQ_H 

#include <isr.h>

/* reference: http://wiki.osdev.org/IRQ */

#define IRQ_TIMER   0
#define IRQ_KB      1
#define IRQ_IDE     14

// Interrupt handling
void irq_handler(struct interrupt_frame *r);
// Register interrupt service routine
void irq_install(uint8_t irq, void (*handler)(struct interrupt_frame *r));
// Unload interrupt service routine
void irq_uninstall(uint8_t irq);
// Initialize interrupt service
void irq_init();

// Allow interrupts
void irq_enable(uint8_t irq);
// Mask interrupt
void irq_disable(uint8_t irq);

// ---------------------------

// Initialize clock
void irq_init_timer(void (*handler)(struct interrupt_frame *r));

#endif
