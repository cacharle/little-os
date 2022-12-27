CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra
AS = nasm
ASFLAGS = -f elf32

OBJ = loader.o main.o

all: os.iso

run: all
	echo 'c' | bochs -f bochsrc.txt -q

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

kernel.elf: $(OBJ) link.ld
	ld -T link.ld -melf_i386 $(OBJ) -o kernel.elf

# loader.o: loader.s
# 	nasm -f elf32 loader.s

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $<

clean:
	-rm *.o
	-rm kernel.elf
	-rm -r iso
