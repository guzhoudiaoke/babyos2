OBJS = \
	cxx.o\
	babyos.o\
	main.o\
	screen.o\
	console.o\
	string.o\
	mm.o\
	arch.o\
	isr.o\
	keyboard.o\
	queue.o\
	timer.o\
	ide.o\
	spinlock.o\
	syscall.o\
	cpu.o\
	i8259a.o\

AS = as --32
CC = gcc -m32
CPP= g++
LD = ld -m elf_i386

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc -fno-stack-protector
LDFLAGS = -m elf_i386 -static

CPPFLAGS = -g -w -trigraphs -fno-builtin -fno-exceptions -fno-stack-protector -m32 -fno-rtti -nostdlib -nodefaultlibs 

all: babyos

babyos: boot loader kernel init
	../tool/checksize loader 1
	../tool/checksize kernel 502
	../tool/checksize init 20
	dd if=boot   of=baby_hd.img ibs=512 count=1 conv=notrunc
	dd if=loader of=baby_hd.img ibs=512 seek=1 count=1 conv=notrunc
	dd if=kernel of=baby_hd.img ibs=512 seek=2 conv=notrunc
	dd if=../res/font/ASC16 of=baby_hd.img ibs=512 seek=504 conv=notrunc
	dd if=init of=baby_hd.img ibs=512 seek=512 conv=notrunc

boot: boot.S
	${CPP} ${CFLAGS} -c boot.S
	${LD} ${LDFLAGS} --oformat binary -N -Ttext 0x7c00 -o boot boot.o

loader: load.S loadmain.cc
	${CPP} ${CFLAGS} -c load.S
	${CPP} ${CFLAGS} -O -c loadmain.cc
	${LD} --oformat binary -N -Ttext 0x0000 -o loader load.o loadmain.o

kernel: ${OBJS} entry.o kernel.ld
	${LD} ${LDFLAGS} -T kernel.ld -o kernel entry.o ${OBJS}

init: init.cc
	${CPP} ${CFLAGS} -O -c init.cc
	${LD} ${LDFLAGS} -Ttext 0x1000 -o init init.o

%.o: %.cc
	$(CPP) $(CPPFLAGS) -c $< -o  $@

%.o: %.S
	$(CPP) $(CPPFLAGS) -c $< -o  $@


clean:
	rm -f boot loader kernel *.o  trace* init

