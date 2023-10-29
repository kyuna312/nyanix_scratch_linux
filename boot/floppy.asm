; Floppy Disk - Boot Sector
INCBIN "bootsect.bin"  ; Boot area code
INCBIN "../bin/kernel" ; kernel code

; other spaces are filled with zeros
; Floppy disk size: 80 (track) x 18 (sector) x 512 bytes (sector size) x 2 (double-sided)
;     = 1440 x 1024 bytes = 1440 KB = 1.44MB
times 80*18*2*512 - ($-$$) db 0 ; 80 (Track / sectors) * 18 (Sector / head) * 2 Cylinder * 512 byte
