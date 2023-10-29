#ifndef __UVM_H
#define __UVM_H

#include <type.h>
#include <vmm.h>
#include <proc.h>

// Kernel space initialization
void kvm_init(pde_t *pgdir);

// User space initialization
void uvm_init(pde_t *pgdir, char *init, uint32_t size); 

// Apply for user space
int uvm_alloc(pte_t *pgdir, uint32_t old_sz, uint32_t new_sz);

// User page table switching
void uvm_switch(struct proc *pp);

// Release user space
void uvm_free(pte_t *pgdir);

// Copy user space
pde_t *uvm_copy(pte_t *pgdir, uint32_t size);

#endif