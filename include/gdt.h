#ifndef __GDT_H 
#define __GDT_H 

#include <type.h>

/* Reference: http://wiki.osdev.org/GDT */

// Global descriptor table structure http://www.cnblogs.com/hicjiajia/archive/2012/05/25/2518684.html
// base: base address
// limit: the maximum addressable range tells the maximum addressable unit
// flags: flag bits see AC_AC above, etc.
struct gdt_entry {
    uint16_t limit_low;       
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    unsigned limit_high: 4;
    unsigned flags: 4;
    uint8_t base_high;
} __attribute__((packed));

// GDTR
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

#define NGDT 256        // Global Descriptor Table Size Global Descriptor Table

#define AC_AC 0x1       // accessible access
#define AC_RW 0x2       // [code] readable; [data] writable readable for code selector & writeable for data selector
#define AC_DC 0x4       // direction bit direction
#define AC_EX 0x8       // executable executable, code segment
#define AC_RE 0x10      // reserved bit reserve
#define AC_PR 0x80      // valid bit persist in memory

// Privileged bit: 01100000b
#define AC_DPL_KERN 0x00 // RING 0 kernel level
#define AC_DPL_SYST 0x20 // RING 1 systask level
#define AC_DPL_USER 0x60 // RING 3 user level

#define GDT_GR  0x8     // Page granularity page granularity, limit in 4k blocks
#define GDT_SZ  0x4     // Size bit size bt, 32 bit protect mode

// gdt selector 
#define SEL_KCODE   0x1 // kernel code segment
#define SEL_KDATA   0x2 // kernel data segment
#define SEL_UCODE   0x3 // User code snippet
#define SEL_UDATA   0x4 // user data segment
#define SEL_SCODE   0x5 // User code snippet
#define SEL_SDATA   0x6 // user data segment
#define SEL_TSS     0x7 // task state segment http://wiki.osdev.org/TSS

// RPL request privilege level request privilege level
#define RPL_KERN    0x0
#define RPL_SYST    0x1
#define RPL_USER    0x3

// CPL current privilege level current privilege level
#define CPL_KERN    0x0
#define CPL_SYST    0x1
#define CPL_USER    0x3

// Initialize GDT
void gdt_init();

#endif
