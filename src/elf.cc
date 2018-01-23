/*
 * guzhoudiaoke@126.com
 * 2017-12-23
 */

#include "elf.h"
#include "babyos.h"
#include "string.h"
#include "math.h"

static int32 read_file_from(int fd, void* buffer, uint32 offset, uint32 size)
{
    if (os()->get_fs()->do_seek(fd, offset) < 0) {
        return -1;
    }

    if (os()->get_fs()->do_read(fd, buffer, size) != size) {
        return -1;
    }

    return 0;
}

static int32 load_elf_binary(elf_hdr_t* elf, int fd)
{
    pde_t* pg_dir = current->m_vmm.get_pg_dir();
    uint32 file_offset = elf->phoff;
    for (int i = 0; i < elf->phnum; i++) {
        /* read prog_hdr */
        prog_hdr_t ph;
        if (read_file_from(fd, &ph, file_offset, sizeof(prog_hdr_t)) != 0) {
            return -1;
        }

        /* check prog hdr type */
        if (ph.type != PT_LOAD) {
            continue;
        }

        void* vaddr = (void*) (ph.vaddr & PAGE_MASK);
        uint32 offset = ph.vaddr - (uint32)vaddr;
        uint32 len = PAGE_ALIGN(ph.memsz + ((uint32)vaddr & (PAGE_SIZE-1)));

        /* mmap */
        int32 ret = current->m_vmm.do_mmap((uint32) vaddr, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED);
        if (ret < 0) {
            return -1;
        }

        /* alloc mem and do map pages */
        void* mem = os()->get_mm()->alloc_pages(math_t::log(2, len / PAGE_SIZE));
        os()->get_mm()->map_pages(pg_dir, vaddr, VA2PA(mem), len, PTE_W | PTE_U);

        /* read data */
        if (read_file_from(fd, mem+offset, ph.off, ph.filesz) != 0) {
            return -1;
        }
        if (ph.memsz > ph.filesz) {
            memset(mem+offset+ph.filesz, 0, ph.memsz - ph.filesz);
        }

        file_offset += sizeof(prog_hdr_t);
    }

    return 0;
}

int32 elf_t::load(trap_frame_t* frame, const char* path)
{
    int ret = 0;

    /* 1. open file */
    int fd = os()->get_fs()->do_open(path, file_t::MODE_RDWR);
    if (fd < 0) {
        console()->kprintf(RED, "BUG on open file %s!\n", path);
        ret = -1;
        goto end;
    }

    // 2. read elf from hard disk
    elf_hdr_t elf; 
    if (os()->get_fs()->do_read(fd, &elf, sizeof(elf_hdr_t)) != sizeof(elf_hdr_t)) {
        ret = -1;
        goto end;
    }

    // 3. check if it's a valid elf file
    if (elf.magic != ELF_MAGIC) {
        console()->kprintf(RED, "BUG on elf format!\n");
        ret = -1;
        goto end;
    }

    // 4. load elf binary
    if (load_elf_binary(&elf, fd) != 0) {
        console()->kprintf(RED, "BUG on load elf binary!\n");
        ret = -1;
        goto end;
    }

    // 5. set eip
    frame->eip = (uint32)(elf.entry);

end:
    // 6. close file
    os()->get_fs()->do_close(fd);

    return ret;
}

