/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#include "types.h"
#include "kernel.h"
#include "elf.h"
#include "x86.h"

extern "C" 
void loadmain(void)
{
    elf_hdr_t *elf = (elf_hdr_t *) (ELF_BASE_ADDR);
    uint8 *base = (uint8 *) elf;

    /* check if it's a valid elf file */
    if (elf->magic != ELF_MAGIC) {
        return;
    }

    /* load program segments */
    prog_hdr_t *ph = (prog_hdr_t *)(base + elf->phoff);
    prog_hdr_t *end_ph = ph + elf->phnum;
    for (; ph < end_ph; ph++) {
        uint8 *pa = (uint8 *)ph->paddr;
        movsb(pa, base + ph->off, ph->filesz);
        if (ph->memsz > ph->filesz) {
            stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }

    /* find entry from elf, and call */
    void (*entry)(void) = (void(*)(void))(elf->entry);
    entry();
}

