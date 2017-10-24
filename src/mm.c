/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#include "kernel.h"
#include "mm.h"

pde_t entry_pg_dir[];
pte_t entry_pte0[];
uint8 kernel_stack[];

__attribute__ ((__aligned__(PAGESIZE)))
uint8 kernel_stack[KSTACK_SIZE] = {
    0xff,
};

__attribute__ ((__aligned__(PAGESIZE)))
pte_t entry_pte0[1024] = { 
    [0] = (0) | PTE_P | PTE_W,
};


__attribute__ ((__aligned__(PAGESIZE)))
pte_t entry_pte_vram[1024] = {
    [0] = (0) | PTE_P | PTE_W,
};

__attribute__ ((__aligned__(PAGESIZE)))
pde_t entry_pg_dir[1024] = { 
    [0] = (0) | PTE_P | PTE_W,
};

void init_mm()
{
}

void kmap_device(void *va)
{
    entry_pg_dir[((uint32)va)>>22] = ((uint32)(VA_2_PA(entry_pte_vram)) | 0x3);
    uint32 entry = ((uint32) va) + 3;
    for (uint32 i = 0; i < 1024; i++) {
        entry_pte_vram[i] = entry;
        entry += 0x1000;
    }
}

