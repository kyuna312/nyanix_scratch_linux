; loader.asm
; jmp to C kernel, achieve some function in asm 
; 

; kernel code segment selector
SEL_KERN_CODE   EQU 0x8
; kernel data segment selector
SEL_KERN_DATA   EQU 0x10
; vedio memory
SEL_KERN_VIDEO  EQU 0x18
; User address starting
USER_BASE       EQU 0xc0000000

align 4

[bits 32]
[section .text]

[extern os_main]
[extern tss_reset]
[extern isr_stub]
[extern k_reenter]
[global start]
start:
    xor eax, eax
    mov ax, SEL_KERN_DATA
    mov ds, ax
    mov ax, SEL_KERN_DATA
    mov es, ax
    mov ax, SEL_KERN_VIDEO
    mov gs, ax
    mov ax, SEL_KERN_DATA
    mov ss, ax
    mov esp, 0x7c00 ; Think of org 0x7c00 in bootsect

; mov the kernel to 0x100000
[extern kernstart]
[extern kernend]
    mov eax, kernend
    mov ecx, kernstart
    sub eax, ecx
    mov ecx, eax
    mov esi, 0x8000
    mov edi, 0x100000
    cld
    rep movsb
    jmp dword SEL_KERN_CODE:go

go:
    mov edi, (160*3)+0   ; 160*50 line 3 column 1
    mov ah, 00001100b    ; red color

    mov esi, msg 
    call print

    push 0
    jmp os_main          ; os entry

    jmp $                ; halt

print:
    add edi, 160
    push edi
    cld

loop:
    lodsb
    cmp al, 0
    je outloop
    mov	[gs:edi], ax
    add edi, 2
    jmp loop

outloop:
    pop edi
    ret

msg:
    db "=== [ OS ENTRY ] ===", 0

; ####################### Kernel-specific #######################

; ********** gdt.c
[global gdt_flush]
[extern gp]

gdt_flush:      
    lgdt [gp] ; load gdt
    mov ax, SEL_KERN_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp SEL_KERN_CODE:flush2
flush2:
    ret

; ********** idt.c
[global idt_load]
[extern idtp]

idt_load:
    lidt [idtp] ; load idt
    ret

; ####################### interrupt service #######################

; ***** Interrupts 0-31 exception int 0 - int 31
%macro m_fault 1
[global fault%1]
fault%1:
    cli
    ; int 8,10-14,17,30 have error code
    %if %1 != 17 && %1 != 30 && (%1 < 8 || %1 > 14)
        push 0 ; fake error code
    %endif
    pop eax
    push %1
    jmp _isr_stub ; Interrupt handler
%endmacro 

%assign i 0
%rep 32
    m_fault i
    %assign i i + 1
%endrep

; ***** Interruption No. 32-47Interrupt Request int 32 - 47
; int 0-7
%macro m_irq 1
[global irq%1]
irq%1:
    push %1+32              ; See Yu Yuan’s minix code
    call save               ; save scene
    in  al, 0x21            ; `.
    or  al, (1 << %1)       ;  | Mask current interrupt
    out 0x21, al            ; /
    mov al, 0x20            ; `. Set EOI bit
    out 0x20, al            ; /
    push %1+32
    mov eax, esi
    push eax
    mov eax, isr_stub
    sti
    call eax                ; Interrupt handling
    cli
    pop ecx
    in  al, 0x21            ; `.
    and al, ~(1 << %1)      ;  | Resume accepting current interrupt
    out 0x21, al            ; /
    add esp, 4
    ret
%endmacro

; int 8-15
%macro m_irq2 1
[global irq%1]
irq%1:
    cli
    push %1+32
    jmp _isr_stub ; Interrupt handler
%endmacro

%assign i 0
%rep 8
    m_irq i
%assign i i+1
%endrep
%rep 8
    m_irq2 i
%assign i i+1
%endrep

; ***** Unknown Interrupt
; 255 is a flag of unknown int
; so please note that we can't use it 
[global isr_unknown]
isr_unknown:
    cli
    push 255
    jmp _isr_stub ; Interrupt handler


; ####################### interrupt service routine #######################

[extern isr_stub]
; a common ISR func, sava the context of CPU 
; call C func to process fault
; at last restore stack frame
_isr_stub:
    push eax

    ; save scene
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, SEL_KERN_DATA ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, esp
    push eax

    mov eax, isr_stub
    call eax ; Interrupt handling
    pop eax

_isr_stub_ret:
    ;restore scene
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

save:
    pushad          ; `.
    push    ds      ;  |
    push    es      ;  | Save original register value
    push    fs      ;  |
    push    gs      ; /
    mov     dx, ss
    mov     ds, dx
    mov     es, dx

    mov     esi, esp                    ; esi = Process table starting address
    
    inc     dword [k_reenter]           ; k_reenter++;
    cmp     dword [k_reenter], 0        ; if (k_reenter == 0)
    jne     .1                          ; {
    mov     ax, SEL_KERN_DATA           ; Switch to kernel data segment
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    push    restart                     ;     push restart
    jmp     [esi + 48]                  ;     return;
.1:                                     ; } else { Already in the kernel stack, no need to switch again
    push    restart_reenter             ;     push restart_reenter
    jmp     [esi + 48]                  ;     return;
                                        ; }

[extern proc]
[global restart]
restart:
    call tss_reset ; Modify tss
    mov eax, [proc]
    mov esp, [eax]  ; esp = proc->fi
restart_reenter:
    dec dword [k_reenter]
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8
    iretd ; Interrupt return, pop the remaining information in the stack, refresh cs and ip

; ####################### system call #######################
; int 0x80
[global _syscall]
_syscall:
    push 0x80
    call save
    sti
    pop eax
    mov [esi + 48], eax ; Save interrupt return address
    mov eax, isr_stub
    push esi
    call eax ; Interrupt handling
    mov [esi + 44], eax ; System call return value
    mov eax, [esi + 48]
    push eax ; Restore the interrupt return address just popped
    cli
    ret

; ####################### interprocess communication #######################
; IPC
[global sendrec]
sendrec:
    mov	eax, 7
    int	0x80
    ret