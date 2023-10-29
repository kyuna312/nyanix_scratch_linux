#include <type.h>
#include <asm.h>
#include <pmm.h>
#include <vmm.h>
#include <idt.h>
#include <isr.h>
#include <proc.h>
#include <sysproc.h>
#include <syscall.h>
#include <print.h>


// system call
extern void _syscall();

// Default call
int sys_none();

// call table
static int (*sys_routines[])(void) = {
    sys_none,
    sys_fork,
    sys_exit,
    sys_exec,
    sys_sleep,
    sys_wait,
    sys_kill,
    sys_ipc,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none,
    sys_none
}; 

/* Call name */
char *sys_name[NSYSCALL + 1] = {
    "sys_none",
    "sys_fork",
    "sys_exit",
    "sys_exec",
    "sys_sleep",
    "sys_wait",
    "sys_kill",
    "sys_ipc",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none",
    "sys_none"
};

// Take an integer from the specified address
int fetchint(uint32_t addr, int *ip) {
    if (addr < USER_BASE
            || addr > USER_BASE + proc->size
            || addr + 4 > USER_BASE + proc->size) {
        return -1;
    }
    *ip = *(int *)(addr);

    return 0;
}

// Get a string from the specified address
int fetchstr(uint32_t addr, char **pp) {
    char *s, *ep;

    if (addr < USER_BASE || addr >= USER_BASE + proc->size) {
        return -1;
    }

    *pp = (char *)addr;
    ep = (char *)(USER_BASE + proc->size);

    for (s = *pp; s < ep; s++) {
        if (*s == '\0') {
            return s - *pp;
        }
    }

    return -1;
}

// Parameters: Take the nth integer
int argint(int n, int *ip) {
    return fetchint(proc->fi->user_esp + 4*n + 4, ip);  // "+4" for eip
}

// Parameters: Take the nth string and return the length
int argstr(int n, char **pp) {
    int addr;

    if (argint(n, &addr) < 0) {
        return -1;
    }
    
    return fetchstr((uint32_t)addr, pp);
}

// Parameters: Get the nth string address
int argptr(int n, char **pp, int size) {
    int addr;

    if(argint(n, &addr) < 0) {
        return -1;
    }

    if ((uint32_t)addr < USER_BASE 
            || (uint32_t)addr >= USER_BASE + proc->size 
            || (uint32_t)addr + size >= USER_BASE + proc->size) {
        return -1;
    }

    *pp = (char *)addr;

    return 0;
}

// Default call
int sys_none() {
    return -1;
}

// Initialization system call
void sys_init() {
    // Register interrupt call
    idt_install(ISR_SYSCALL, (uint32_t)_syscall, SEL_KCODE << 3, GATE_TRAP, IDT_PR | IDT_DPL_SYST);
}

// system call
void syscall() {
    int cn;

    if (!proc || proc->state == P_SLEEPING) {
        proc->fi->eax = -1;
        return;
    }

    cn = proc->fi->eax; // eaxRepresents the calling number
    
    //printk("proc#%d: %s\n", proc->pid, sys_name[cn]);

    if (cn > 0 && cn <= NSYSCALL && sys_routines[cn]) {
        proc->fi->eax = sys_routines[cn]();
    } else {
        proc->fi->eax = -1;
    }
}

// http://www.cnblogs.com/taek/archive/2012/02/05/2338838.html
int call(int no) {
    volatile int ret = -1;
    __asm__ __volatile__("mov 0x8(%ebp), %eax\n\t"
        "int $0x80\n\t"
        "mov %eax, -0x4(%ebp)");
    return ret;
}
