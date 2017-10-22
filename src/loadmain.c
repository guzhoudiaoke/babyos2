/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#include "types.h"
#include "kernel.h"
#include "elf.h"
#include "x86.h"


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

void set_pixel(int32 x, int32 y, uint8 r, uint8 g, uint8 b)
{
    uint8* pvram = NULL;

    pvram = p_vram_base_addr + n_bytes_per_pixel*y*cx_screen + n_bytes_per_pixel*x;
    pvram[0] = b;
    pvram[1] = g;
    pvram[2] = r;
}

void memcpy(void *dst, void *src, uint32 count)
{
    uint8 *d = (uint8 *) dst;
    uint8 *s = (uint8 *) src;
    for (uint32 i = 0; i < count; i++) {
        d[i] = s[i];
    }
}

void loadmain(void)
{
    p_vram_base_addr = p_video_info->p_vram_base_addr;

    elf_hdr_t *elf = (elf_hdr_t *) (TMP_KERNEL_ADDR + 2*SECT_SIZE);
    uint8 *base = (uint8 *) elf;

    // Is this an ELF executable?
    if (elf->magic != ELF_MAGIC) {
        for (int i = 0x100; i < 0x120; i++) {
            set_pixel(i, 0x100, 0xff, 0x00, 0x00);
        }
        return;
    }

    // Load each program segment (ignores ph flags).
    prog_hdr_t *ph = (prog_hdr_t *)(base + elf->phoff);
    prog_hdr_t *end_ph = ph + elf->phnum;
    for (; ph < end_ph; ph++) {
        uint8 *pa = (uint8 *)ph->paddr;
        memcpy(pa, base + ph->off, ph->filesz);


        if (ph->memsz > ph->filesz) {
            stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }


    for (int i = 0x100; i < 0x120; i++) {
        set_pixel(i, 0x120, 0x00, 0xff, 0x00);
    }


    // Call the entry point from the ELF header.
    // Does not return!
    void (*entry)(void) = (void(*)(void))(elf->entry);
    entry();
}

