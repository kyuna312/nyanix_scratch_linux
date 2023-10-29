# Nyannix

## C based mini operating system test

======================================
Simple x86-based OS.

## development stage

1. **[0.2.0]** Start from scratch, keep the core files, and slowly add functions to realize Hello world
2. **[0.2.1]** add GDT and IDT
3. **[0.2.2]** add ISR and IRQ
4. **[0.2.3]** add PMM
5. **[0.2.4]** add VMM
6. **[0.2.5]** Add multi-process mechanism and improve comments
7. **[0.2.6]** Implement inter-process communication IPC

## build tools

* make
* nasm
* gcc
* binutils
* cgdb
* qemu

```bash
sudo apt install make nasm gcc binutils cgdb qemu
sudo ln -s /usr/bin/qemu-system-i386 /usr/bin/qemu
```

## compile and run

```shell
make init   # only for first time
make fs     # build root file system and user routines, root privilege required
make        # build kernel
make run    # run with qemu
```
