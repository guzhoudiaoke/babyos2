/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#include "types.h"
#include "elf.h"
#include "x86.h"


/* GDT和IDT内存地址和大小 */
#define	IDT_ADDR			(0x90000)	
#define	IDT_SIZE			(256*8)
#define	GDT_ADDR			(IDT_ADDR + IDT_SIZE)
#define	GDT_LEN				(5)
#define	GDT_SIZE			(8 * GDT_LEN)

/* 显示模式的一些信息的内存地址 */
#define	VIDEO_INFO_ADDR		(GDT_ADDR + GDT_SIZE)

typedef struct vidoe_info_s {
    uint16 video_mode;
    uint16 cx_screen;
    uint16 cy_screen;
    uint8  n_bits_per_pixel;
    uint8  n_memory_model;
    uint8* p_vram_base_addr;
} video_info_t;

video_info_t* p_video_info = (video_info_t *)VIDEO_INFO_ADDR;

uint8* p_vram_base_addr = (uint8 *)0xe0000000;
uint32 cx_screen = 1024;
uint32 cy_screen = 768;
uint32 n_bytes_per_pixel = 3;

static bool is_pixel_valid(int32 x, int32 y)
{
    if (x < 0 || y < 0 || (uint32)x >= cx_screen || (uint32)y >= cy_screen) {
        return false;
    }

    return true;
}

bool set_pixel(int32 x, int32 y, uint8 r, uint8 g, uint8 b)
{
    uint8* pvram = NULL;
    if (!is_pixel_valid(x, y)) {
        return false;
    }

    pvram = p_vram_base_addr + n_bytes_per_pixel*y*cx_screen + n_bytes_per_pixel*x;
    pvram[0] = b;
    pvram[1] = g;
    pvram[2] = r;

    return true;
}

void test()
{

    for (int i = 100; i < 1024-100; i++) {
        set_pixel(i, 200, 0xff, 34, 89);
    }
}

void bootmain(void)
{
    p_vram_base_addr = p_video_info->p_vram_base_addr;

    test();

#if 0
    elf_hdr_t  *elf;
    prog_hdr_t *ph, *eph;
    uint8* pa;

    elf = (elf_hdr_t *)0x10000;

    // Is this an ELF executable?
    if (elf->magic != ELF_MAGIC) {
        return;
    }

    // Load each program segment (ignores ph flags).
    ph = (prog_hdr_t *)((uint8 *)elf + elf->phoff);
    eph = ph + elf->phnum;
    for(; ph < eph; ph++){
        pa = (uint8 *)ph->paddr;
        //read_seg(pa, ph->filesz, ph->off);
        //if(ph->memsz > ph->filesz)
        //    stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
    }

    // Call the entry point from the ELF header.
    // Does not return!
    void (*entry)(void) = (void(*)(void))(elf->entry);
    entry();
#endif
}

