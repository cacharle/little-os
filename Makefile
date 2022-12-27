
all: os.iso

os.iso: kernel.elf
	mkdir -p iso/boot/grub
	cp stage2_eltorito iso/boot/grub/
	cp kernel.elf iso/boot/
	cp menu.lst iso/boot/grub/
	genisoimage \
		-R \
		-b boot/grub/stage2_eltorito \
		-no-emul-boot \
		-boot-load-size 4 \
		-A os \
		-input-charset utf8 \
		-quiet \
		-boot-info-table \
		-o os.iso \
		iso

kernel.elf: loader.o link.ld
	ld -T link.ld -melf_i386 loader.o -o kernel.elf

loader.o: loader.s
	nasm -f elf32 loader.s

clean:
	rm loader.o
	rm kernel.elf
	rm -r iso
