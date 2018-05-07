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
	timer.o\
	spinlock.o\
	syscall.o\
	cpu.o\
	i8259a.o\
	vm.o\
	pool.o\
	elf.o\
    process.o\
    signal.o\
    waitqueue.o\
	fs.o\
	file.o\
    sem.o\
    block_dev.o\
	hd.o\
	io_buffer.o\
	inode.o\
	fs_test.o\
	file_table.o\
	pipe.o\
	sys_socket.o\
	socket.o\
	socket_local.o\
	math.o\
	local_apic.o\
	i8254.o\
	io_apic.o\
	mp_config.o\
	timer_mgr.o\
	process_mgr.o\
	delay.o\
	pci.o\
	rtl8139.o\
	net.o\
	ethernet.o\
	arp.o\
	ip.o\
	icmp.o\
	sock_ring_buffer.o\
	sock_addr.o\
	socket_raw.o\
    net_buf.o\
	udp.o\
	dns.o\
	socket_dgram.o\
	tcp.o\
	socket_stream.o\

AS = as --32
CC = gcc -m32
CPP= g++
LD = ld

CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc -fno-stack-protector
LDFLAGS = -m elf_i386 -static

CPPFLAGS = -g -w -trigraphs -fno-builtin -fno-exceptions -fno-stack-protector -m32 -fno-rtti -nostdlib -nodefaultlibs -Werror -fno-omit-frame-pointer 

all: babyos

babyos: boot loader kernel init shell ls cat ln mkdir rm cp mv ping
	../tool/checksize boot 1
	../tool/checksize loader 2
	../tool/checksize kernel 2037
	dd if=boot              of=baby_hd.img ibs=512 seek=0   count=1 conv=notrunc
	dd if=loader            of=baby_hd.img ibs=512 seek=1   count=2 conv=notrunc
	dd if=kernel            of=baby_hd.img ibs=512 seek=3           conv=notrunc
	dd if=../res/font/ASC16 of=baby_hd.img ibs=512 seek=2040        conv=notrunc
	../tool/mkfs ./init ./shell ./ls ./cat ./ln ./mkdir ./rm ./cp ./mv ./ping
	cp fs.img fs1.img
	cp baby_hd.img baby_hd1.img

boot: boot.S
	${CPP} ${CPPFLAGS} -c boot.S
	${LD} ${LDFLAGS} --oformat binary -N -Ttext 0x7c00 -o boot boot.o

loader: load.S loadmain.cc
	${CPP} ${CPPFLAGS} -c load.S
	${CPP} ${CPPFLAGS} -O -c loadmain.cc
	${LD} ${LDFLAGS} --oformat binary -N -Ttext 0x0000 -o loader load.o loadmain.o

start_ap: start_ap.S
	${CPP} ${CPPFLAGS} -c start_ap.S
	${LD} ${LDFLAGS} --oformat binary -N -Ttext 0x9000 -o start_ap start_ap.o

kernel: ${OBJS} start_ap entry.o kernel.ld
	${LD} ${LDFLAGS} -T kernel.ld -o kernel entry.o ${OBJS} -b binary start_ap

userlib.o: userlib.cc
	${CPP} ${CPPFLAGS} -O -c userlib.cc

init: init.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c init.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o init init.o userlib.o

shell: shell.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c shell.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o shell shell.o userlib.o

ls: ls.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c ls.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o ls ls.o userlib.o

ln: ln.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c ln.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o ln ln.o userlib.o

rm: rm.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c rm.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o rm rm.o userlib.o

cat: cat.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c cat.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o cat cat.o userlib.o

mkdir: mkdir.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c mkdir.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o mkdir mkdir.o userlib.o

cp: cp.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c cp.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o cp cp.o userlib.o

mv: mv.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c mv.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o mv mv.o userlib.o

ping: ping.cc userlib.o
	${CPP} ${CPPFLAGS} -O -c ping.cc
	${LD} ${LDFLAGS} -e main -Ttext 0x1000 -o ping ping.o userlib.o

%.o: %.cc
	$(CPP) $(CPPFLAGS) -c $< -o  $@

%.o: %.S
	$(CPP) $(CPPFLAGS) -c $< -o  $@


clean:
	rm -f boot loader kernel *.o  trace* init shell ls cat cp ln mkdir mv cp rm ping start_ap

