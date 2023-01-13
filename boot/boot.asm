[org 0x00]
[bits 16]

section code

.init:
    mov eax, 0x07c0
    mov ds, eax
    mov eax, 0xb800
    mov es, eax
    mov eax, 0 ; set eax to 0
    mov ebx, 0 ; index of the character in the string that we are printing.
    mov ecx, 0 ; actual address of the character on the screen
    mov dl, 0  ; store the actual value

.clear:
    mov byte [es:eax], 0 ; Move blank character to current text address
    inc eax
    mov byte [es:eax], 0xb0 ; Move the background colour and character colour to the next address
    inc eax 

    cmp eax, 2 * 25 * 80

    jl .clear

mov eax, text

mov ecx, 3 * 2 * 80

push .end
call .print

.end:
     mov byte [es:0x00], 'N'
     jmp $ 

.print:
    mov dl, byte [eax + ebx]
   
    cmp dl, 0 
    je .print_end

    mov byte [es:ecx], dl
    
    inc ebx
    inc ecx
    inc ecx

    jmp .print

;.main:
;    mov byte [es:0x00], 'N' ; 0xb800 + 0x00 = 0xb800
;    mov byte [es:0x01], 0x30


.print_end:
    ret
    ;mov eax, 0

text: db 'Nyannix OS', 0
text1: db 'This is Test Game OS!!!!', 0

times 510 - ($ - $$) db 0x00 ; Pads the file with 0x, making it the right size

db 0x55
db 0xaa
    
