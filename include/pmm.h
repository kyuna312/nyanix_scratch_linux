#ifndef __PMM_H
#define __PMM_H

/* page memory management */
//Memory physical page frame allocation

#define STACK_SIZE 8192
extern uint32_t kern_stack_top;

#define ADDR_RANGE_MEMORY   1
#define ADDR_RANGE_RESERVED 2
#define ADDR_RANGE_UNDEFINE 3

#define PMM_MAX_SIZE 0x04000000 // 64M
#define PMM_PAGE_SIZE 0x1000
#define PAGE_MAX_SIZE (PMM_MAX_SIZE/PMM_PAGE_SIZE)
#define PMM_PAGW_MASK 0xfffff000

// ARD Address Range Descriptor
// size = 20
struct ard_entry {
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
}__attribute__((packed));

// The page frame here is a physical page frame, corresponding to the physical address, not the virtual address

// Page frame initialization
void pmm_init();

//Physical page table size
uint32_t pmm_size();

// Application frame
uint32_t pmm_alloc();

// free page frame
void pmm_free(uint32_t addr);

#endif