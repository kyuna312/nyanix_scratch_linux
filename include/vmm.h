#ifndef __VMM_H
#define __VMM_H

/* virtual memory management */
// Virtual memory allocation (secondary page table allocation method)

// Reference: http://wiki.osdev.org/Paging

// For a 32-bit virtual address
// 32-22: Page directory number | 21-12: Page table number | 11-0: Intra-page offset
// http://www.360doc.com/content/11/0804/10/7204565_137844381.shtml

/* 4k per page */
#define PAGE_SIZE 4096 

/* Page mask, take the high 20 bits */
#define PAGE_MASK 0xfffff000

/* Address alignment */
#define PAGE_ALIGN_DOWN(x) ((x) & PAGE_MASK)
#define PAGE_ALIGN_UP(x) (((x) + PAGE_SIZE - 1) & PAGE_MASK)

/* Analyze address */
#define PDE_INDEX(x) (((x) >> 22) & 0x3ff)  // Get the page directory number corresponding to the address
#define PTE_INDEX(x) (((x) >> 12) & 0x3ff)  // Get the page table number
#define OFFSET_INDEX(x) ((x) & 0xfff)       // Get the page offset


//Page directory entries and page table entries can be represented by uint32
typedef uint32_t pde_t;
typedef uint32_t pte_t;

/* Page directory size 1024 */
#define PDE_SIZE (PAGE_SIZE/sizeof(pte_t))
/* Page table size 1024 */
#define PTE_SIZE (PAGE_SIZE/sizeof(pde_t))
/* Total number of page tables 1024*PTE_SIZE*PAGE_SIZE = 4G */
#define PTE_COUNT 1024

/* CPU */
#define CR0_PG  0x80000000 

/* pde&pdt attribute */
#define PTE_P   0x1     // Valid bits Present
#define PTE_R   0x2     // Read/Write, can be read&write when set
#define PTE_U   0x4     // User Bit User / Kern
#define PTE_K   0x0     // Kernel Bits User / Kern
#define PTE_W   0x8     // Write through
#define PTE_D   0x10    // Cache disable
#define PTE_A   0x20    // Accessed
#define PTE_S   0x40    // Page size, 0 for 4kb pre page
#define PTE_G   0x80    // Ignored

/* User space base address */
#define USER_BASE 0xc0000000

// Initialize page table
void vmm_init();

// enable paging
void vmm_switch(uint32_t pgd);

// virtual page mapping
void vmm_map(pde_t *pgdir, uint32_t va, uint32_t pa, uint32_t flags);

// Unmap
void vmm_unmap(pde_t *pgdir, uint32_t va);

// Query paging status
int vmm_ismap(pde_t *pgdir, uint32_t va, uint32_t *pa);

#endif