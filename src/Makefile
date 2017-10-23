OBJS = \
	main.o\
	screen.o\
	console.o\
	string.o\
	mm.o\

AS = as --32
CC = gcc -m32
LD = ld -m elf_i386

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc -fno-stack-protector -O
LDFLAGS = -m elf_i386

all: babyos

babyos: boot loader kernel
	../tool/checksize loader 1
	../tool/checksize kernel 128
	dd if=boot   of=baby_hd.img ibs=512 count=1 conv=notrunc
	dd if=loader of=baby_hd.img ibs=512 seek=1 count=1 conv=notrunc
	dd if=kernel of=baby_hd.img ibs=512 seek=2 conv=notrunc
	dd if=../res/font/ASC16 of=baby_hd.img ibs=512 seek=130 conv=notrunc

boot: boot.s
	${CC} ${CFLAGS} -c boot.s
	${LD} ${LDFLAGS} --oformat binary -N -Ttext 0x7c00 -o boot boot.o

loader: load.s loadmain.c
	${CC} ${CFLAGS} -c load.s
	${CC} ${CFLAGS} -c loadmain.c
	${LD} --oformat binary -N -Ttext 0x0000 -o loader load.o loadmain.o

kernel: ${OBJS} entry.o kernel.ld
	${LD} ${LDFLAGS} -T kernel.ld -o kernel entry.o ${OBJS}

clean:
	rm -f boot loader kernel *.o  trace*
