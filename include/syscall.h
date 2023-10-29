#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <type.h>
#include <isr.h>

#define NSYSCALL    20

// The call number is set by yourself (in future microkernel architecture, system calls will be converted into signals)
// Call number: mov eax #n; int 0x80
#define SYS_FORK    1
#define SYS_EXIT    2
#define SYS_EXEC    3
#define SYS_SLEEP   4
#define SYS_WAIT    5
#define SYS_KILL    6
#define SYS_IPC     7 //IPC implements microkernel


// Specify the address to take an integer
int fetchint(uint32_t addr, int *ip);

// Get the string from the specified address
int fetchstr(uint32_t addr, char **pp);

// Parameter: take the nth integer
int argint(int n, int *ip);

// Parameters: take the nth string and return the length
int argstr(int n, char **pp);

// Parameter: Get the nth string address
int argptr(int n, char **pp, int size);

// Initialization system call
void sys_init();

// system call
void syscall();

//User call
int call(int no);

#endif
