 /* idt.c 
  * This file is modified form Bram's Kernel Development Tutorial
  * install idt, 256 entrys table
  */

#include <type.h>
#include <string.h>
#include <print.h>
#include <idt.h>
#include <proc.h>
#include <uvm.h>
#include <vmm.h>

static struct idt_entry idt[NIDT];
struct tss_entry tss;
struct idt_ptr idtp; // used in loader.asm

extern void idt_load(); // Implemented in loader.asm

void idt_install(uint8_t num, uint32_t base, uint16_t selctor, uint8_t gate, uint8_t flags) {
    /* The interrupt routine's base address */
    idt[num].base_low = (base & 0xffff);
    idt[num].base_high = (base >> 16) & 0xffff;

    /* The segment or 'selector' that this IDT entry will use
    *  is set here, along with any access flags */
    idt[num].selector = selctor; 
    idt[num].always0 = 0; 
    idt[num].gate_type = 0x0f & gate;
    idt[num].flags = 0x0f & flags;
}

void idt_init() {
    idtp.limit = (sizeof (struct idt_entry) * NIDT) - 1;
    idtp.base = (uint32_t)&idt;

    /* Clear out the entire IDT, initializing it to zeros */    
    memset(&idt, 0, sizeof(struct idt_entry) * NIDT);

    /* Add any new ISRs to the IDT here using idt_install */
    idt_load();
}

// Set task status segment
void tss_install() {
    __asm__ volatile("ltr %%ax" : : "a"((SEL_TSS << 3)));
}

// Set TSS
void tss_set(uint16_t ss0, uint32_t esp0) {
    // Clear TSS
    memset((void *)&tss, 0, sizeof(tss));
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.iopb_off = sizeof(tss);
}

// Reset the TSS of the current process
void tss_reset() {
    // TSS is used to set the stack when switching to ring0
    // Each process has a kernel stack
    tss_set(SEL_KDATA << 3, (uint32_t)proc->stack + PAGE_SIZE);
}
