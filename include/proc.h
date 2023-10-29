#ifndef __PROC_H
#define __PROC_H

#include <type.h>
#include <vmm.h>

// Maximum length of process name
#define PN_MAX_LEN 16

// Maximum number of processes
#define NPROC      128

/* Process status */

#define P_UNUSED    0x0 // Unused
#define P_USED      0x1 // Used
#define P_RUNABLE   0x2 // Runable
#define P_RUNNING   0x3 // Running
#define P_SLEEPING  0x4 // Sleeping
#define P_ZOMBIE    0x5 // Zombie

// Priority is time slice allocation
#define PRIOR_SYST  0x20 // System service priority
#define PRIOR_USER  0x10 // User process priority

/**
 * MESSAGE structure / borrowed from MINIX
 */
struct msg1 {
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
};
struct msg2 {
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
};
struct msg3 {
    int	m3i1;
    int	m3i2;
    int	m3i3;
    int	m3i4;
    u64	m3l1;
    u64	m3l2;
    void* m3p1;
    void* m3p2;
};
typedef struct {
    int source;
    int type;
    union {
        struct msg1 m1;
        struct msg2 m2;
        struct msg3 m3;
    } u;
} MESSAGE;

// PCB process control block v
// http://blog.csdn.net/wyzxg/article/details/4024340
struct proc {
    /* KERNEL */
    struct interrupt_frame *fi;     // interrupt scene
    volatile uint8_t pid;           // Process ID
    uint32_t size;                  // User space size
    uint8_t state;                  // process status
    char name[PN_MAX_LEN];          // process name
    pde_t *pgdir;                   // Virtual page directory (first-level page table)
    char *stack;                    // Process kernel stack
    struct proc *parent;            // parent process
    int8_t ticks;                   // time slice
    int8_t priority;                // priority
    /* IPC */
    int p_flags;                    // logo
    MESSAGE *p_msg;                 // information
    int p_recvfrom;                 // The process ID that received the message
    int p_sendto;                   // The process ID that sent the message
    int has_int_msg;                // nonzero if an INTERRUPT occurred when the task is not ready to deal with it.
    struct proc *q_sending;         // queue of procs sending messages to this proc
    struct proc *next_sending;      // next proc in the sending queue (q_sending)
};

// currently running process
extern struct proc *proc;

// Record the number of interruption reentries
extern int32_t k_reenter;

// clock
extern uint32_t tick;

// Initialization process management
void proc_init();

// process scheduling loop
void schedule();

// Process forking
int fork();

// Wait for child process
int wait();

// Process sleeps
void sleep();

// process wake up
void wakeup(uint8_t pid);

// kill process
int kill(uint8_t pid);

// Process exits
void exit();

// ---------------------------------

#define proc2pid(x) ((x) -> pid)

struct proc *nproc(int offset);
struct proc *npid(int pid);
int npoffset(int pid);

void* va2la(int pid, void* va);

#endif
