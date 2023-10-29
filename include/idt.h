#ifndef __IDT_H
#define __IDT_H

#include <type.h>
#include <gdt.h>

/* reference: http://wiki.osdev.org/IDT */

// Interrupt descriptor table structure http://blog.csdn.net/fwqcuc/article/details/5855460
// base: Store the address of the interrupt handler/interrupt vector
// selector: Interrupt vector selector, the DPL of its selector must be zero (that is, the highest level ring0) http://blog.csdn.net/better0332/article/details/3416749
// gate: reference http://wiki.osdev.org/IDT
//       0b0101	0x5	5	80386 32 bit task gate
//       0b0110	0x6	6	80286 16-bit interrupt gate
//       0b0111	0x7	7	80286 16-bit trap gate
//       0b1110	0xE	14	80386 32-bit interrupt gate
//       0b1111	0xF	15	80386 32-bit trap gate
// flags:
//       Storage Segment(0) Set to 0 for interrupt gates (see above).
//       DPL(1-2) Gate call protection. Privilege level 0-3
//       P(3) Set to 0 for unused interrupts.
struct idt_entry
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;        // must be 0
    unsigned gate_type : 4; // gate type
    unsigned flags : 4;     // S(0) DPL(1-2) P(3)
    uint16_t base_high;
} __attribute__((packed));

// IDTR
struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// 任务状态段 task state segment http://wiki.osdev.org/TSS
// The only interesting fields are SS0 and ESP0.
//   SS0 gets the kernel datasegment descriptor (e.g. 0x10 if the third entry in your GDT describes your kernel's data)
//   ESP0 gets the value the stack-pointer shall get at a system call
//   IOPB may get the value sizeof(TSS) (which is 104) if you don't plan to use this io-bitmap further (according to mystran in http://forum.osdev.org/viewtopic.php?t=13678)

// http://blog.csdn.net/huybin_wang/article/details/2161886
// The use of TSS is to solve the changes in the stack when the privilege level changes in the call gate.

// http://www.kancloud.cn/wizardforcel/intel-80386-ref-manual/123838
/*
    The TSS status segment consists of two parts:
        1. Dynamic part (the processor will set these field values every time the task switches)
            General registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
            Segment registers (ES, CS, SS, DS, FS, GS)
            Status register (EFLAGS)
            Instruction Pointer (EIP)
            Selector of the TSS segment of the previously executed task (only updated when returning)
        2. Static fields (read by the processor but never changed)
            Task's LDT selector
            Page Directory Base Register (PDBR) (read-only when paging is enabled)
            Inner stack pointer, privilege level 0-2
            T-bit indicates whether the processor raises a debug exception when switching tasks
            I/O bitmap base address
*/
struct tss_entry
{
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint16_t padding1;
    uint16_t iopb_off;
} __attribute__((packed));

#define NIDT 256 // Interrupt Descriptor Table Size Interrupt Descriptor Table

/* 386 32-bit gata type */
#define GATE_TASK 0x5
#define GATE_INT 0xe
#define GATE_TRAP 0xf

#define IDT_SS 0x1       // storage segment store segment
#define IDT_DPL_KERN 0x0 // kernel privilege level descriptor privilege level
#define IDT_DPL_SYST 0x2 // system service privilege level descriptor privilege level
#define IDT_DPL_USER 0x6 // user privilege level descriptor privilege level
#define IDT_PR 0x8       // Valid bit present in memory

// Initialize interrupt vector table
void idt_init();

// Set interrupt vector
void idt_install(uint8_t num, uint32_t base, uint16_t selector, uint8_t gate, uint8_t flags);

// Set task status segment
void tss_install();

// Set TSS to the current process stack
extern void tss_reset();

#endif
