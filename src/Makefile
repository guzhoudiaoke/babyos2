AS = as --32
CC = gcc -m32
LD = ld -m elf_i386

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc 

all: babyos

babyos: boot loader
	dd if=boot of=baby_hd.img ibs=512 count=1 conv=notrunc
	dd if=loader of=baby_hd.img ibs=512 seek=1 count=2 conv=notrunc

boot: boot.s
	${CC} ${CFLAGS} -c boot.s
	${LD} --oformat binary -N -Ttext 0x7c00 -o boot boot.o

loader: load.s bootmain.c
	${CC} ${CFLAGS} -c load.s
	${CC} ${CFLAGS} -c bootmain.c
	${LD} --oformat binary -N -Ttext 0x0000 -o loader load.o bootmain.o

clean:
	rm -f boot loader *.o
