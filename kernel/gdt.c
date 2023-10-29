/* gdt.c 
 * This file is modified form Bram's Kernel Development Tutorial
 * set the new gdt, the new gdt table has 256 entrys
 */

#include <type.h>
#include <gdt.h>
#include <idt.h>
#include <string.h>

extern struct tss_entry tss;

static struct gdt_entry gdt[NGDT]; // 256 gdt entry
struct gdt_ptr gp; // used in loader.asm

extern void gdt_flush(); // Implement extern func in loader.asm

void gdt_install(uint8_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {

    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xffff);
    gdt[num].base_middle = (base >> 16) & 0xff;
    gdt[num].base_high = (base >> 24) & 0xff;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xffff);
    gdt[num].limit_high = ((limit >> 16) & 0x0f);

    /* Finally, set up the granularity and access flags */
    gdt[num].flags = flags;

    access |= AC_RE; // Set reserved bit to 1
    gdt[num].access = access;
}

void tss_init() {
    gdt_install(SEL_TSS, (uint32_t)&tss, sizeof(tss), AC_PR|AC_AC|AC_EX, GDT_GR); 
    /* for tss, access_reverse bit is 1 */
    gdt[SEL_TSS].access &= ~AC_RE;
}

void gdt_init() {
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * NGDT) - 1;
    gp.base = (uint32_t)&gdt;

    // Note: After enabling paging GDT_GR, the unit of limit is 4KB, so 0xfffff*4KB=4GB

    /* null descriptor */
    gdt_install(0, 0, 0, 0, 0);
    /* kernel code segment type: code addr: 0 limit: 4G gran: 4KB sz: 32bit */
    gdt_install(SEL_KCODE, 0, 0xfffff, AC_RW|AC_EX|AC_DPL_KERN|AC_PR, GDT_GR|GDT_SZ);
    /* kernel data segment type: data addr: 0 limit: 4G gran: 4KB sz: bit 32bit */
    gdt_install(SEL_KDATA, 0, 0xfffff, AC_RW|AC_DPL_KERN|AC_PR, GDT_GR|GDT_SZ);
    /* user code segment type: code addr: 0 limit: 4G gran: 4KB sz: 32bit */
    gdt_install(SEL_UCODE, 0, 0xfffff, AC_RW|AC_EX|AC_DPL_USER|AC_PR, GDT_GR|GDT_SZ);
    /* user code segment type: data addr: 0 limit: 4G gran: 4KB sz: 32bit */
    gdt_install(SEL_UDATA, 0, 0xfffff, AC_RW|AC_DPL_USER|AC_PR, GDT_GR|GDT_SZ);
    /* systask code segment type: code addr: 0 limit: 4G gran: 4KB sz: 32bit */
    gdt_install(SEL_SCODE, 0, 0xfffff, AC_RW|AC_EX|AC_DPL_SYST|AC_PR, GDT_GR|GDT_SZ);
    /* systask code segment type: data addr: 0 limit: 4G gran: 4KB sz: 32bit */
    gdt_install(SEL_SDATA, 0, 0xfffff, AC_RW|AC_DPL_SYST|AC_PR, GDT_GR|GDT_SZ);

    tss_init();
    gdt_flush();
    tss_install();
}
