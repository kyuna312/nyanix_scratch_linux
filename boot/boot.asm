[org 0x7c00]
[bits 16]

section code

.switch:
    mov ax, 0x4f01 ; querying the VBE
    mov cx, 0x111 ; Mode we want
    mov bx, 0x0800 ; Offset for the vbe info structure
    mov es, bx
    mov di, 0x00
    int 0x10

    ; Make the switch to graphics mode
    mov ax, 0x4f02
    mov bx, 0x111
    int 0x10

    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, 0x1000 ; This is the location where the code is loaded from hard disk
    mov ah, 0x02
    mov al, 50 ; The number of sectors to read from hard disk
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02
    int 0x13

    cli ; Turh off the interrupts
    lgdt [gdt_descriptor] ; Load the GDT Table

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax ; Make the switch

    jmp code_seg:protected_start

welcome: db 'Welcome to SaphireOS.', 0

[bits 32]
protected_start:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Update the stack pointer
    mov ebp, 0x90000
    mov esp, ebp

    call 0x1000
    jmp $

gdt_begin:
gdt_null_descriptor:
    dd 0x00
    dd 0x00
gdt_code_seg:
    dw 0xeeee
    dw 0x00
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
gdt_data_seg:
    dw 0xeeee
    dw 0x00
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_begin - 1
    dd gdt_begin

code_seg equ gdt_code_seg - gdt_begin
data_seg equ gdt_data_seg - gdt_begin

times 510 - ($ - $$) db 0x00 ; Pads the file with 0s, making the file the right size

db 0x55
db 0xaa