/* pmm.h
  * Memory physical page allocation
  */
 
#include <type.h>
#include <string.h>
#include <pmm.h>

extern uint8_t kernstart; // kernel start address
extern uint8_t code;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t kernend;   // Kernel end address


static uint32_t pmm_stack[PAGE_MAX_SIZE + 1]; // Available memory table
static uint32_t pmm_stack_top = 0; // top of stack
static uint32_t pmm_count = 0;
static uint32_t mem_size = 0;

void pmm_init() {
    // ARD_count and ARD are calculated in bootsect.asm
    uint32_t ARD_count = *(uint32_t *)0x400;
    struct ard_entry *ARD = (struct ard_entry *)0x500;
    uint32_t i = 0;

    for (i = 0; i < ARD_count; i++) {

       if (ARD[i].type == ADDR_RANGE_MEMORY && // If the memory segment is valid
           ARD[i].base_addr_low == 0x100000) { // And the base address is 0x100000

           uint32_t addr = ((uint32_t)&kernend + PMM_PAGE_SIZE) & 0xfffff000;
           uint32_t limit = ARD[i].base_addr_low + ARD[i].len_low;

           while (addr < limit && addr <= PMM_MAX_SIZE) {
               mem_size += PMM_PAGE_SIZE;
               pmm_stack[pmm_stack_top++] = addr; // Map free memory into pmm_stack
               addr += PMM_PAGE_SIZE;
               pmm_count++;
           }
       }
    }
}

uint32_t pmm_size() {
    return mem_size;
}

uint32_t pmm_alloc() {
    uint32_t addr = pmm_stack[--pmm_stack_top];
    memset((void *)addr, 0, PMM_PAGE_SIZE); // The application content is cleared
    return addr;
}

void pmm_free(uint32_t addr) {
    pmm_stack[pmm_stack_top++] = addr;
    memset((void *)addr, 1, PMM_PAGE_SIZE);
}
