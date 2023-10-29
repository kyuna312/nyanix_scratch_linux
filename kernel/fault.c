#include <type.h>
#include <vga.h>
#include <idt.h>
#include <isr.h>
#include <fault.h>
#include <print.h>

/* int 0-31 is used to service exceptions! */
extern void fault0();
extern void fault1();
extern void fault2();
extern void fault3();
extern void fault4();
extern void fault5();
extern void fault6();
extern void fault7();
extern void fault8();
extern void fault9();
extern void fault10();
extern void fault11();
extern void fault12();
extern void fault13();
extern void fault14();
extern void fault15();
extern void fault16();
extern void fault17();
extern void fault18();
extern void fault19();
extern void fault20();
extern void fault21();
extern void fault22();
extern void fault23();
extern void fault24();
extern void fault25();
extern void fault26();
extern void fault27();
extern void fault28();
extern void fault29();
extern void fault30();
extern void fault31();

static char *fault_msg[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void fault_init()
{
    idt_install(0, (uint32_t)fault0, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(1, (uint32_t)fault1, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(2, (uint32_t)fault2, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(3, (uint32_t)fault3, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(4, (uint32_t)fault4, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(5, (uint32_t)fault5, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(6, (uint32_t)fault6, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(7, (uint32_t)fault7, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);

    idt_install(8, (uint32_t)fault8, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(9, (uint32_t)fault9, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(10, (uint32_t)fault10, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(11, (uint32_t)fault11, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(12, (uint32_t)fault12, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(13, (uint32_t)fault13, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(14, (uint32_t)fault14, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(15, (uint32_t)fault15, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);

    idt_install(16, (uint32_t)fault16, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(17, (uint32_t)fault17, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(18, (uint32_t)fault18, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(19, (uint32_t)fault19, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(20, (uint32_t)fault20, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(21, (uint32_t)fault21, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(22, (uint32_t)fault22, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(23, (uint32_t)fault23, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);

    idt_install(24, (uint32_t)fault24, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(25, (uint32_t)fault25, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(26, (uint32_t)fault26, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(27, (uint32_t)fault27, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(28, (uint32_t)fault28, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(29, (uint32_t)fault29, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(30, (uint32_t)fault30, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);
    idt_install(31, (uint32_t)fault31, SEL_KCODE << 3, GATE_INT, IDT_PR|IDT_DPL_KERN);

}

static void page_fault(struct interrupt_frame *r) {
    uint32_t cr2;

    __asm__ volatile ("mov %%cr2, %0":"=r"(cr2));

    printk("Page fault! [addr: 0x%x]\n", cr2);
    panic(fault_msg[r->int_no], r->cs & 0x3);
}

void fault_handler(struct interrupt_frame *r) {

    // Page Fault
    if (r->int_no == 14) { // Page error
        page_fault(r);
    }

    if (r->int_no < 32) {
        printk("System fault!\n");
        panic(fault_msg[r->int_no], r->cs & 0x3);
    } else {
        panic("Unknown Interrupt, System Halted!\n", r->cs & 0x3);
    }
}
