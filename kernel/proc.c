#include <type.h>
#include <debug.h>
#include <asm.h>
#include <gdt.h>
#include <idt.h>
#include <pmm.h>
#include <vmm.h>
#include <uvm.h>
#include <isr.h>
#include <ipc.h>
#include <string.h>
#include <print.h>
#include <proc.h>

#define PROC_IS_RUNABLE(pp) ((pp)->state == P_RUNABLE && (pp)->p_flags == 0)

// Interrupt reentrancy count
int32_t k_reenter = 0;

// Process ID (incremented each time, used to assign to new process)
static uint32_t cur_pid = 0;

// clock
uint32_t tick = 0;

// Process PCB array
static struct proc pcblist[NPROC];

// current process
struct proc *proc = NULL;

// open up new process
static struct proc *proc_alloc() {
    struct proc *pp;
    char *p;

    // Find idle processes
    for (pp = &pcblist[0]; pp < &pcblist[NPROC]; pp++) {

        if (pp->state == P_UNUSED) { // Find unused processes

            /*
                ------------------------------ 4KB
                interrupt_frame
                ...
                stack
                ------------------------------ 0KB
            */

            pp->state = P_USED; // occupy it
            pp->pid = ++cur_pid; // Increment process ID

            pp->stack = (char *)pmm_alloc(); // Apply for physical page frame

            // The p pointer points to the page header p = page_head
            p = pp->stack;

            // p = page_head + page size - interrupt site structure size
            p = p + PAGE_SIZE - sizeof(*pp->fi);

            // //Interrupt site fi position of the process = page_head + page size - interrupt site structure size
            pp->fi = (struct interrupt_frame *)p;

            // Initialization priority
            pp->priority = PRIOR_USER;

            // Reset time slice
            pp->ticks = 0;

            // init ipc data
            pp->p_flags = 0;
            pp->p_recvfrom = TASK_NONE;
            pp->p_sendto = TASK_NONE;
            pp->has_int_msg = 0;
            pp->q_sending = 0;
            pp->next_sending = 0;

            return pp;
        }
    }
    return 0; // Failure to create returns empty
}

// Initialization process management
void proc_init() {
    struct proc *pp;
    extern char __init_start;
    extern char __init_end;

    // PCB array clear
    memset(pcblist, 0, sizeof(struct proc) * NPROC);

    // Initialize the first process
    pp = proc_alloc();

    // Initialize the first-level page table
    pp->pgdir = (pde_t *)pmm_alloc();

    // Initialize the kernel stack
    kvm_init(pp->pgdir);

    // Mapping the kernel stack
    vmm_map(pp->pgdir, (uint32_t)pp->stack, (uint32_t)pp->stack, PTE_P | PTE_R | PTE_K);

    // get kernel size
    uint32_t size = &__init_end - &__init_start;

    // User space size is 4KB

    pp->size = PAGE_SIZE;

    // Copy kernel data to mapped user space
    uvm_init(pp->pgdir, &__init_start, size);

    // Copy the interrupted scene and restore _isr_stub_ret after execution.
    pp->fi->cs = (SEL_SCODE << 3) | RPL_SYST; // 0x1 RPL=1
    pp->fi->ds = (SEL_SDATA << 3) | RPL_SYST; // SYS TASK
    pp->fi->es = pp->fi->ds;
    pp->fi->fs = pp->fi->ds;
    pp->fi->gs = pp->fi->ds;
    pp->fi->ss = pp->fi->ds;
    pp->fi->eflags = 0x202;
    pp->fi->user_esp = USER_BASE + PAGE_SIZE; // The top of the stack because it grows downwards
    pp->fi->eip = USER_BASE; // Execute starting from 0xc0000000

    strcpy(pp->name, "init"); // The first process is named init

    pp->state = P_RUNABLE; // Status set to runnable

    pp->priority = PRIOR_SYST; // Initialization priority

    proc = pp;
}

// Process switching
static void proc_switch(struct proc *pp) {
    if (proc->state == P_RUNNING) {
        proc->state = P_RUNABLE;
    }
    pp->state = P_RUNABLE;
    proc = pp; // Switch the currently active process
}

// Select available processes
static struct proc *proc_pick() {
    struct proc *pp, *pp_ready;
    int greatest_ticks = 0;

    // Find available processes
    for (pp = &pcblist[0]; pp < &pcblist[NPROC]; pp++) {

        if (PROC_IS_RUNABLE(pp)) { // The process is active and not blocked

            if (pp->ticks > greatest_ticks) {
                greatest_ticks = pp->ticks;
                pp_ready = pp;
            }
        }
    }

    if (!greatest_ticks)
        return 0;
    return pp_ready;
}

// Reset time slice
static void reset_time() {
    int i;
    struct proc *pp;
    i = 0;
    for (pp = &pcblist[0]; pp < &pcblist[NPROC]; pp++) {
        if (PROC_IS_RUNABLE(pp)) {
            i++;
            pp->ticks = pp->priority;
        }
    }
    if (i == 0) {
        assert(!"no runable process!");
    }
}

// process scheduling
void schedule() {
    struct proc *pp, *pp_ready;
    pp_ready = 0;

    while (!pp_ready) {
        
        pp_ready = proc_pick(); // selection process

        if (!pp_ready) { // All process time slices are used up and reset
            reset_time();
        } else {
            pp = pp_ready;

            // Turn off interrupts
            cli();

            uvm_switch(pp); // Switch page table
            proc_switch(pp); // Switch process

            // enable interrupt
            sti();

            return;
        }
    }
}

// Process sleeps
void sleep() {
    cli();

    if (proc->state == P_RUNNING) {
        proc->state = P_SLEEPING;
    }

    sti();
}

// wake up process
void wakeup(uint8_t pid) {
    struct proc* pp;

    cli();

    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {
        if (pp->pid == pid && pp->state == P_SLEEPING) {
            pp->state = P_RUNABLE;
            return;
        }
    }

    sti();
}

// process destroyed
static void proc_destroy(struct proc *pp) {

    pmm_free((uint32_t)pp->stack); // Release stack memory
    pp->stack = 0;
    uvm_free(pp->pgdir); // free page table

    pp->state = P_UNUSED;
    pp->pid = 0;
    pp->parent = 0;
    pp->name[0] = 0;
}

// Wait for child process
int wait() {
    uint8_t has_child, pid;
    struct proc* pp;

    cli();

    has_child = 0;

    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {
        if (pp->parent != proc) {
            continue; // no child process
        }

        has_child = 1;

        if (pp->state == P_ZOMBIE) {

            pid = pp->pid;
            proc_destroy(pp);

            sti();

            return pid; // Returns the ID of the child process that just exited
        }
    }

    if (!has_child || proc->state == P_ZOMBIE) {
        sti();
        return -1; // No need to wait for a process to be killed
    }

    sleep(); // Wait, you cannot block here because you need to prevent interruption from re-entry.

    sti();

    return -1;
}

// Exit process
void exit() {
    struct proc *pp;

    cli();

    if (!proc->parent) { // init
        sti();
        return;
    }

    wakeup(proc->parent->pid); // Wake up the parent process of the current process
    proc->state = P_ZOMBIE; // Marked as a zombie process, waiting for the parent process to wait to be destroyed
    
    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {
        if (pp->parent == proc) { // Find child process
            pp->parent = proc->parent; // Set the parent process of the child process to the parent process of the current process (current process skip)
            if (pp->state == P_ZOMBIE) { // If the child process is a zombie process
                wakeup(proc->parent->pid); // wakeup(proc->parent->pid); // Wake up the parent process of the current process
            }
        }
    }

    sti();
}

// kill process
int kill(uint8_t pid) {
    struct proc *pp;

    cli();

    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {

        if (pp->pid == pid) { // Find the process to kill
            pp->state = P_ZOMBIE; // Mark as zombie process
            sti();
            return 0;
        }
    }

    sti();

    return -1;
}

// Process forking
int fork() {
    struct proc *child; // child process

    cli();

    if ((child = proc_alloc()) == 0) {
        sti();
        return -1; // Creating process failed
    }

    // Copy the user space of the current process to the child process
    child->pgdir = uvm_copy(proc->pgdir, proc->size);

    // map stack
     /* The lack of this sentence is the culprit causing the bug
      * The specific reason is: unmapped memory causes inaccessibility, which in turn causes system reset
    */
    vmm_map(child->pgdir, (uint32_t)child->stack, (uint32_t)child->stack, PTE_P | PTE_R | PTE_U);

    if (child->pgdir == 0) { // Copy failed
        panic("fork:", child->fi->cs & 0x3);
        pmm_free((uint32_t)child->stack);
        child->stack = 0;
        child->state = P_UNUSED;
        sti();
        return -1; // Creating process failed
    }

    child->size = proc->size;   // User space consistent
    child->parent = proc;       // Set the parent process as the current process
    *child->fi = *proc->fi;     // copy interruption scene

    child->fi->eax = 0;

    strncpy(child->name, proc->name, sizeof(proc->name)); // Copy process name

    child->state = P_RUNABLE; // Set status to runnable

    sti();
    
    return child->pid; // Return child process PID
}

void irq_handler_clock(struct interrupt_frame *r) {

    tick++;

    if (!proc)
        return;

    if (PROC_IS_RUNABLE(proc)) {
        if (proc->ticks > 0) {
            proc->ticks--;
            return;
        }

        if (k_reenter != 0) { // Prevent reentrancy
            return;
        }
    }
    
    schedule();
}

struct proc *nproc(int offset) {
    return &pcblist[offset];
}

struct proc *npid(int pid) {
    struct proc* pp;

    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {
        if (pp->pid == pid) {
            return pp;
        }
    }
    return 0;
}

int npoffset(int pid) {
    int i;

    for (i = 0; i < NPROC; i++) {
        if (pcblist[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

void* va2la(int pid, void* va) {
    struct proc* pp;

    for (pp = pcblist; pp < &pcblist[NPROC]; pp++) {
        if (pp->pid == pid) {

            uint32_t pa;
            
            if (vmm_ismap(pp->pgdir, (uint32_t)va, &pa)) {
                return (void*)((uint32_t)pa + OFFSET_INDEX((uint32_t)va));
            }

            assert(!"vmm: va not mapped!");

            return 0;
        }
    }

    return 0;
}
