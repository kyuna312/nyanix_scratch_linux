; Reference<Orange's>

; Descriptor Type
DA_32       EQU 0x4000  ; 32 Rank
DA_LIMIT_4K EQU 0x8000  ; Segment boundary granularity is 4K bytes

DA_DPL0     EQU   0x00  ; DPL = 0
DA_DPL1     EQU   0x20  ; DPL = 1
DA_DPL2     EQU   0x40  ; DPL = 2
DA_DPL3     EQU   0x60  ; DPL = 3

; bucket descriptor type
DA_DR       EQU 0x90    ; Existing read-only data segment type value
DA_DRW      EQU 0x92    ; Existing readable and writable data segment attribute values
DA_DRWA     EQU 0x93    ; Existing accessed read-write data segment type value
DA_C        equ 0x98    ; Existing execution-only code segment attribute value
DA_CR       equ 0x9a    ; The existing executable and readable code segment attribute value
DA_CCO      equ 0x9c    ; Exists only execute consistent code segment attribute value
DA_CCOR     equ 0x9e    ; Existing executable readable consistent code segment attribute value

; System segment descriptor type
DA_LDT      EQU   0x82  ; 局部描述符表段类型值
DA_TaskGate EQU   0x85  ; 任务门类型值
DA_386TSS   EQU   0x89  ; 可用 386 任务状态段类型值
DA_386CGATE EQU   0x8c  ; 386 调用门类型值
DA_386IGATE EQU   0x8e  ; 386 中断门类型值
DA_386TGATE EQU   0x8f  ; 386 陷阱门类型值


; Selector Attribute
SA_RPL0     EQU 0   ; ┓
SA_RPL1     EQU 1   ; ┣ RPL(Request Privilege Level)
SA_RPL2     EQU 2   ; ┃
SA_RPL3     EQU 3   ; ┛

SA_TIG      EQU 0   ; ┓TI(Table Indicator)
SA_TIL      EQU 4   ; ┛

; constant for Pageing
PG_P        EQU 1   ; Page exists attribute bit
PG_RWR      EQU 0   ; R/W attribute bit value, read/execute
PG_RWW      EQU 2   ; R/W attribute bit value, read/write/execute
PG_USS      EQU 0   ; U/S attribute bit value, system level
PG_USU      EQU 4   ; U/S attribute bit value, user level

; usage: Descriptor Base, Limit, Attr
;        Base:  dd
;        Limit: dd (low 20 bits available)
;        Attr:  dw (lower 4 bits of higher byte are always 0)
%macro Descriptor 3
    DW  %2 & 0xffff                         ; Segment limit 1
    DW  %1 & 0xffff                         ; Segment base address 1
    DB  (%1 >> 16) & 0xff                   ; Segment base address 2
    DW  ((%2 >> 8) & 0xf00) | (%3 & 0xf0ff) ; Attribute 1 + Segment Boundary 2 + Attribute 2
    DB  (%1 >> 24) & 0xff                   ; Segment base address 3
%endmacro ; 共 8 字节


; usage: Gate Selector, Offset, DCount, Attr
;        Selector:  dw
;        Offset:    dd
;        DCount:    db
;        Attr:      db
%macro Gate 4
    DW  (%2 & 0xffff)                       ; Offset1 偏移1
    DW  %1                                  ; Selector 选择子
    DW  (%3 & 0x1f) | ((%4 << 8) & 0xff00)  ; Attr 属性
    DW  ((%2 >> 16) & 0xffff)               ; Offset2 偏移2
%endmacro ; 共 8 字节

; usage segment reg, selector, gdt entry
%macro FillDesc 3
    xor eax, eax
    mov ax, %1 
    shl eax, 4
    add eax, %2 
    mov word [%3 + 2], ax
    shr eax, 16
    mov byte [%3 + 4], al
    mov byte [%3 + 7], ah
%endmacro
