/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#include "types.h"
#include "kernel.h"
#include "elf.h"
#include "x86.h"

#define HD_STATE_READY  0x40
#define HD_STATE_BUSY   0x80
#define IO_CMD_READ     0x20


typedef void (*kernel_entry_t)(void);

void wait_disk()
{
    while ((inb(0x1f7) & (HD_STATE_BUSY | HD_STATE_READY)) != HD_STATE_READY) {
        ;
    }
}

void read_sector(void* buf, uint32 lba)
{
    wait_disk();

    outb(0x1f2, 1);                     /* sector num */
    outb(0x1f3, lba & 0xff);
    outb(0x1f4, (lba >> 8)  & 0xff);
    outb(0x1f5, (lba >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((lba >> 24) & 0xff));
    outb(0x1f7, IO_CMD_READ);

    wait_disk();
    insl(0x1f0, buf, SECT_SIZE / 4);
}

/* pa: the buffer to read data, will be aligned by SECT_SIZE
 * offset: where to read from disk (byte)
 * size: how many byte to read
 */
void read_segment(void* pa, uint32 offset, uint32 size)
{
    uint8* p = (uint8 *) pa - (offset % SECT_SIZE);;
    uint32 lba = offset / SECT_SIZE;
    uint8* end = p + size;

    for (; p < end; p += SECT_SIZE, lba++) {
        read_sector(p, lba);
    }
}

extern "C" 
void loadmain()
{
    char buf[512] = {0};
    elf_hdr_t* elf = (elf_hdr_t *) buf;
    read_sector(elf, KERNEL_ELF_LBA);
    if (elf->magic != ELF_MAGIC) {
        return;
    }

    /* read segments */
    uint32 elf_offset = SECT_SIZE * KERNEL_ELF_LBA;
    prog_hdr_t* ph = (prog_hdr_t *) ((uint8 *)elf + elf->phoff);
    for (int i = 0; i < elf->phnum; i++, ph++) {
        read_segment((void *) ph->paddr, elf_offset + ph->off, ph->filesz);
        if (ph->memsz > ph->filesz) {
            stosb((void *) ph->paddr + ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }

    /* load font */
    uint8* font_addr = (uint8 *) FONT_ASC16_ADDR;
    uint32 font_lba = FONT_ASC16_LBA;
    for (int i = 0; i < FONT_ASC16_SECT_NUM; i++, font_addr += SECT_SIZE, font_lba++) {
        read_sector(font_addr, font_lba);
    }

    /* find entry from elf, and call */
    kernel_entry_t entry = (kernel_entry_t) elf->entry;
    entry();
}

