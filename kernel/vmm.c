/* vmm.h
  * Virtual memory allocation
  */

#include <type.h>
#include <string.h>
#include <pmm.h>
#include <vmm.h>

extern uint8_t kernstart;
extern uint8_t kernend;

/* Kernel page table = PTE_SIZE*PAGE_SIZE */
pde_t pgd_kern[PTE_SIZE] __attribute__((aligned(PAGE_SIZE)));
/* Kernel page table content = PTE_COUNT*PTE_SIZE*PAGE_SIZE */
pte_t pte_kern[PTE_COUNT][PTE_SIZE] __attribute__((aligned(PAGE_SIZE)));

/* Modify CR3 register */
inline void vmm_switch(uint32_t pde) {
    __asm__ volatile ("mov %0, %%cr3": :"r"(pde));
}

/* Refresh TLB */
// http://blog.csdn.net/cinmyheart/article/details/39994769
static inline void vmm_reflush(uint32_t va) {
    __asm__ volatile ("invlpg (%0)"::"a"(va));
}

//Allow paging
// http://blog.csdn.net/whatday/article/details/24851197
static inline void vmm_enable() {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= CR0_PG;
	__asm__ volatile ("mov %0, %%cr0" : : "r" (cr0));
}

void vmm_init() {
    uint32_t i;

    // map 4G memory, physcial address = virtual address
    for (i = 0; i < PTE_SIZE; i++) {
        pgd_kern[i] = (uint32_t)pte_kern[i] | PTE_P | PTE_R | PTE_K;
    }
    
    uint32_t *pte = (uint32_t *)pte_kern;
    for (i = 1; i < PTE_COUNT*PTE_SIZE; i++) {
        pte[i] = (i << 12) | PTE_P | PTE_R | PTE_K; // i is the page table number
    }

    vmm_switch((uint32_t)pgd_kern);
    vmm_enable();
}

// virtual page mapping
// va = virtual address pa = physical address
void vmm_map(pde_t *pgdir, uint32_t va, uint32_t pa, uint32_t flags) {
    uint32_t pde_idx = PDE_INDEX(va); // page catalog number
    uint32_t pte_idx = PTE_INDEX(va); // Page table number

    pte_t *pte = (pte_t *)(pgdir[pde_idx] & PAGE_MASK); // page table

    if (!pte) { // Page missing
        if (va >= USER_BASE) { // If it is a user address, convert it
            pte = (pte_t *)pmm_alloc(); // Apply for a physical page frame to be used as a new page table
            pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // Set page table
            pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // Set page table entry
        } else { // Kernel address not translated
            pte = (pte_t *)(pgd_kern[pde_idx] & PAGE_MASK); // Get kernel page table
            pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // Set page table
        }
    } else { // pte exists
        pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // Set page table entry
    }
}

// Free virtual pages
void vmm_unmap(pde_t *pde, uint32_t va) {
    uint32_t pde_idx = PDE_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pde[pde_idx] & PAGE_MASK);

    if (!pte) {
        return;
    }

    pte[pte_idx] = 0; // Clear the page table entry, the valid bit is zero at this time
    vmm_reflush(va); // Refresh TLB
}

// Is it paginated?
int vmm_ismap(pde_t *pgdir, uint32_t va, uint32_t *pa) {
    uint32_t pde_idx = PDE_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t *pte = (pte_t *)(pgdir[pde_idx] & PAGE_MASK);
    if (!pte) {
        return 0; // Page table does not exist
    }
    if (pte[pte_idx] != 0 && (pte[pte_idx] & PTE_P) && pa) {
        *pa = pte[pte_idx] & PAGE_MASK; // Computational physics page
        return 1; // Page exists
    }
    return 0; // Page table entry does not exist
}