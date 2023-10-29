#include <type.h>
#include <idt.h>
#include <pmm.h>
#include <string.h>
#include <uvm.h>
#include <proc.h>

/* Map all memory from 0 - USER_BASE(0xc0000000) into the page table */
void kvm_init(pde_t *pgdir) {
    uint32_t addr;

    for (addr = 0; addr < pmm_size(); addr += PAGE_SIZE) {
        // http://www.cnblogs.com/Acg-Check/p/4268136.html
        vmm_map(pgdir, addr, addr, PTE_P | PTE_R | PTE_K); // Originally the kernel space, it was temporarily set to PTE_U in order to implement the microkernel.
    }
}

/* Mapping space */
void uvm_init(pde_t *pgdir, char *init, uint32_t size) {
    char *room;
    room = (char *)pmm_alloc(); // Application frame
    memcpy(room, init, size); // copy initial value
    vmm_map(pgdir, USER_BASE, (uint32_t)room, PTE_U | PTE_P | PTE_R); // User space base address stores initial value
}

// Page table switching
void uvm_switch(struct proc *pp) {
    /* Note: According to the implementation of the Linux kernel, tss does not participate in process switching
      * However, the role of tss is to restore ss0 and esp0 when the privilege level is switched.
      * Since this sentence was deleted previously, the user mode jumps directly to int 0x80
      * The 0x00000000 address, which is the BIOS code, causes a bug
      *
      * The cause of the BUG has been identified: the page table setting in the next sentence is defective, that is, there is no mapping stack pp->stack
      * The TSS initialization statement has been placed in proc_init
    */
    //Switch to process page table
    vmm_switch((uint32_t)pp->pgdir);
}

/* Release space */
void uvm_free(pte_t *pgdir) {
    uint32_t i;

    for (i = PDE_INDEX(USER_BASE); i < PTE_COUNT; i++) {
        if (pgdir[i] & PTE_P) { // efficient
            pmm_free(pgdir[i] & PAGE_MASK); // Release physical page
        }
    }

    pmm_free((uint32_t)pgdir); // free page table
}

// copy space
pde_t *uvm_copy(pte_t *pgdir, uint32_t size) {
    pde_t *pgd;
    uint32_t i, pa, mem;

    pgd = (pde_t *)pmm_alloc(); // Apply for physical page

    kvm_init(pgd); // Map space to physical pages

    for (i = 0; i < size; i += PAGE_SIZE) {
        if (vmm_ismap(pgdir, USER_BASE + i, &pa)) {
            mem = pmm_alloc();
            memcpy((void *)mem, (void *)pa, PAGE_SIZE);
            vmm_map(pgd, USER_BASE + i, mem, PTE_R | PTE_U | PTE_P);
        }
    }

    return pgd;
}

// Apply for space
int uvm_alloc(pte_t *pgdir, uint32_t old_sz, uint32_t new_sz) {
    uint32_t mem;
    uint32_t start;

    if (new_sz < old_sz) { // The new space is smaller, no need to apply
        return old_sz;
    }

    // Map excess space
    for (start = PAGE_ALIGN_UP(old_sz); start < new_sz; start += PAGE_SIZE) {
        mem = pmm_alloc();
        vmm_map(pgdir, start, mem, PTE_P | PTE_R | PTE_U);
    }

    return new_sz;
}
