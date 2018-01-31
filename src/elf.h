/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"
#include "traps.h"

#define ELF_MAGIC   0x464c457FU   // "\x7FELF"
#define PT_LOAD     1

// File header
typedef struct elf_hdr_s {
    uint32 magic;  // must equal ELF_MAGIC
    uint8  elf[12];
    uint16 type;
    uint16 machine;
    uint32 version;
    uint32 entry;
    uint32 phoff;
    uint32 shoff;
    uint32 flags;
    uint16 ehsize;
    uint16 phentsize;
    uint16 phnum;
    uint16 shentsize;
    uint16 shnum;
    uint16 shstrndx;
} elf_hdr_t;

// Program section header
typedef struct prog_hdr_s {
    uint32 type;
    uint32 off;
    uint32 vaddr;
    uint32 paddr;
    uint32 filesz;
    uint32 memsz;
    uint32 flags;
    uint32 align;
} prog_hdr_t;


class elf_t {
public:
    static int32 load(trap_frame_t* frame, const char* path);
};

#endif

