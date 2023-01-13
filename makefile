all: bootloader

bootloader:
	mkdir boot/bin
	nasm boot/boot.asm -f bin -o boot/bin/boot.bin
	nasm boot/kernel_entry.asm -f macho -o boot/bin/kernel_entry.bin
	
	gcc -m32 -ffreestanding -c boot/main.c -o boot/bin/kernel.o
	ld -m elf_i386 -o boot/bin/kernel.img -Ttext 0x1000 boot/bin/kernel_entry.bin boot/bin/kernel.o

	objcopy -O binary -j .text boot/bin/kernel.img boot/bin/kernel.bin
	cat boot/bin/boot.bin boot/bin/kernel.bin > os.img

clear:
	rm -f boot/boot.img

run:
	qemu-system-x86_64 -drive format=raw. file=os.img
