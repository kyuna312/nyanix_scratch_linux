; Boot sector FAT12

%INCLUDE "gdt.asm" ; segment

[BITS 16]
org 0x7c00 ; Address унших 

BS_jmpBoot      jmp   entry
                db    0x90
BS_OEMName      db    "CCOSV587"        ; OEM name / 8 B
BPB_BytsPerSec  dw    512               ; 512 bytes ийн сектор
BPB_SecPerClus  db    1                 ; кластер бүрт 1 сектор
BPB_RsvdSecCnt  dw    1                 ; sector ийн тоо 1
BPB_NumFATs     db    2                 ; FAT table
BPB_RootEntCnt  dw    224               ; root directory оролтийн тоо
BPB_TotSec16    dw    2880              ; RolSec16, sectors секторийн нийт тоо
BPB_Media       db    0xf0              ; Media Type: Removable Media
BPB_FATSz16     dw    9                 ; Number of sectors occupied by FATSz16 partition table
BPB_SecPerTrk   dw    18                ; SecPerTrk, диск
BPB_NumHeads    dw    2                 ; толгойн тоо
BPB_HiddSec     dd    0                 ; HiddSec
BPB_TotSec32    dd    2880              ; Card багтаамж
BS_DrvNum       db    0                 ; DvcNum
BS_Reserved1    db    0                 ; NT reserved
BS_BootSig      db    0x29              ; BootSig extended boot tag
BS_VolD         dd    0xffffffff        ; VolID 
BS_VolLab       db    "FLOPPYCDDS "     ; label
BS_FileSysType  db    "FAT12   "        ; FilesysType

times 18 db 0

_print16:
loop:
    lodsb   ; ds:si -> al
    or al,al
    jz done
    mov ah,0x0e
    mov bx,15        
    int 0x10        ; хэвлэх
    jmp loop
done:
    ret

;============================================================
; Оролт
entry:
    mov ax,0        
    mov ss,ax
    mov sp,0x7c00
    mov ds,ax
    mov es,ax   ; bios interrupt expects ds

    ; shift to text mode, 16 color 80*25 
    mov ah,0x0
    mov al,0x03  ; Setting Mode 16 Colors 80x25 Matrix
    int 0x10     ; set color

    mov si, msg_boot
    call _print16

;============================================================
; get memory layout

ARD_ENTRY equ 0x500
ARD_COUNT equ 0x400

get_mem_map:
    mov dword [ARD_COUNT], 0
    mov ax, 0
    mov es, ax
    xor ebx, ebx           ; If this is the first call, EBX must contain zero.
    mov di, ARD_ENTRY      ; ES:DI-> Buffer Pointer

get_mem_map_loop:
    mov eax, 0xe820        ; code
    mov ecx, 20            ; Buffer Size, min = 20
    mov edx, 0x534D4150    ; Signature, edx = "SMAP"
    int 0x15
    jc get_mem_map_fail    ; Non-Carry - indicates no error
    add di, 20             ; Buffer Pointer += 20(sizeof structure)
    inc dword [ARD_COUNT]  ; Inc ADR count
    cmp ebx, 0             ; Anything else?
    jne get_mem_map_loop 
    jmp get_mem_map_ok

get_mem_map_fail:
    mov si, msg_get_mem_map_err
    call _print16
    hlt

get_mem_map_ok:

;============================================================
; Read kernel code from floppy disk

; When reading the disk, put the read sectors into the memory starting at [es:bx]
; When writing to disk, write a sector starting with [es:bx] to the disk
; In both places, [es:bx] is called the data buffer

; read 20 sector (360 KB) form floppy
loadloader:      
    mov bx,0    
    mov ax,0x0800 
    mov es,ax   ; [es:bx] buffer address point -> 0x8000 Store the read data to 0x8000
    mov cl,2    ; Sector
    mov ch,0    ; Track
    mov dh,0    ; Cylinder
    mov dl,0    ; driver a:
    ; kernel locates after bootloader, which is the second sector

readloop:
    mov si,0    ; err counter

retry:
    mov ah,0x02 ; int 0x13 ah = 0x02 read sector form dirve
    mov al,1    ; read 1 sector 
    int 0x13    ; read track 1
    jnc next    ; Continue reading without errors
    add si,1
    cmp si,5    ; If the cumulative error occurs 5 times, an error will be reported
    jae error
    mov ah,0
    mov dl,0    ; driver a
    int 0x13    ; reset
    jmp next 

next: 
    mov ax,es
    add ax,0x20 ; A sector is 512B=0x200, es is a segment, so divide it by 16 to get 0x20
    mov es,ax

    add cl,1    ; read next sector sector + 1
    cmp cl,18   ; 18 sector If all 18 sectors are read, then
    jbe readloop

    mov cl,1
    add dh,1    ; disk + 1
    cmp dh,1
    jbe readloop

    mov dh,0
    add ch,1    ; Track + 1
    cmp ch,20   ; Only read 20 tracks total 360KB
    jbe readloop
    jmp succ

error:        
    mov  si,msg_err ; report an error
    call _print16
    jmp $           ; halt

succ:    
    mov si,msg_succ ; read successfully
    call _print16

    ; Read Global Descriptor Table Register
    xor eax,eax
    mov ax,ds
    shl eax,4
    add eax,GDT                 ; eax <- gdt base 
    mov dword [GdtPtr+2],eax    ; [GdtPtr + 2] <- gdt base 

    lgdt [GdtPtr]
    cli

    ; turn on A20 line
    in al,0x92
    or al,00000010b
    out 0x92,al

    ; shift to protect mode
    mov eax,cr0
    or eax,1
    mov cr0,eax

    ; special, clear pipe-line and jump
    ; Read the floppy disk data to 0x8000 before, now jump to 0x8000
    jmp dword Selec_Code32_R0:0x8000 ; Automatically switch segment registers

msg_boot:
    db "[Bootsector] loading...",13,10,0 ; 13 10(0x0D 0x0A)is '\r \n'
msg_err:
    db "[Bootsector] error",13,10,0
msg_succ:
    db "[Bootsector] ok",13,10,0
msg_temp:
    db 0,0,0
msg_get_mem_map_err:
    db "[Bootsector] failed",0

GDT: ; global descriptor table
DESC_NULL:        Descriptor 0,       0,            0                         ; null
DESC_CODE32_R0:   Descriptor 0,       0xfffff - 1,  DA_C+DA_32+DA_LIMIT_4K    ; uncomfirm 
DESC_DATA_R0:     Descriptor 0,       0xfffff - 1,  DA_DRW+DA_32+DA_LIMIT_4K  ; uncomfirm  ; 4G seg 
DESC_VIDEO_R0:    Descriptor 0xb8000, 0xffff,       DA_DRW+DA_32              ; vram 

GdtLen  equ $ - GDT     ; GDT len
GdtPtr  dw  GdtLen - 1  ; GDT limit
        dd  0           ; GDT Base

; GDT Selector 
Selec_Code32_R0 equ     DESC_CODE32_R0 - DESC_NULL
Selec_Data_R0   equ     DESC_DATA_R0   - DESC_NULL 
Selec_Video_R0  equ     DESC_VIDEO_R0  - DESC_NULL

times 510 - ($-$$) db 0 ; 填充零
db 0x55, 0xaa
