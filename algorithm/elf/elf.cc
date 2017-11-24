#include "elf.h"
#include <cstdio>

elf_t::elf_t(string name)
{
	FILE* fp = fopen(name.c_str(), "r");
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	m_file_buf = new uint8[size];
	fread(m_file_buf, sizeof(uint8), size, fp);

	fclose(fp);
}

elf_t::~elf_t()
{
	delete m_file_buf;
}

void elf_t::dump()
{
    elf_hdr_t* hdr = (elf_hdr_t *) (m_file_buf);
    uint8* base = (uint8 *) m_file_buf;

    // check if it's a valid elf file
    if (hdr->magic != ELF_MAGIC) {
        return;
    }

    // load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + hdr->phoff);
    prog_hdr_t *end_ph = ph + hdr->phnum;
    for (; ph < end_ph; ph++) {
		if (ph->type == 1) {
			printf("off:%x vaddr:%x paddr:%x filesz:%x memsz:%x flags:%x align:%x\n", 
				ph->type, ph->off, ph->vaddr, ph->paddr,
				ph->filesz, ph->memsz, ph->flags, ph->align);
		}
    }
}

void elf_t::dump1()
{
    elf_hdr_t* hdr = (elf_hdr_t *) (m_file_buf);
    uint8* base = (uint8 *) m_file_buf;

    // check if it's a valid elf file
    if (hdr->magic != ELF_MAGIC) {
        return;
    }

    // load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + hdr->phoff);
    prog_hdr_t *end_ph = ph + hdr->phnum;
    for (; ph < end_ph; ph++) {
		printf("type:%x, off:%x vaddr:%x paddr:%x filesz:%x memsz:%x flags:%x align:%x\n", 
			ph->type, ph->off, ph->vaddr, ph->paddr,
			ph->filesz, ph->memsz, ph->flags, ph->align);
    }
}


void elf_t::dump2()
{
    elf_hdr_t* hdr = (elf_hdr_t *) (m_file_buf);
    uint8* base = (uint8 *) m_file_buf;

    // check if it's a valid elf file
    if (hdr->magic != ELF_MAGIC) {
        return;
    }

    // load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + hdr->phoff);
    prog_hdr_t *end_ph = ph + hdr->phnum;
    for (; ph < end_ph; ph++) {
		printf("type:%x, off:%x vaddr:%x paddr:%x filesz:%x memsz:%x flags:%x align:%x\n", 
			ph->type, ph->off, ph->vaddr, ph->paddr,
			ph->filesz, ph->memsz, ph->flags, ph->align);
    }
}

void elf_t::dump3()
{
    elf_hdr_t* hdr = (elf_hdr_t *) (m_file_buf);
    uint8* base = (uint8 *) m_file_buf;

    // check if it's a valid elf file
    if (hdr->magic != ELF_MAGIC) {
        return;
    }

    // load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + hdr->phoff);
    prog_hdr_t *end_ph = ph + hdr->phnum;
    for (; ph < end_ph; ph++) {
		printf("type:%x, off:%x vaddr:%x paddr:%x filesz:%x memsz:%x flags:%x align:%x\n", 
			ph->type, ph->off, ph->vaddr, ph->paddr,
			ph->filesz, ph->memsz, ph->flags, ph->align);
    }
}

int main()
{
	elf_t elf("./kernel");
	elf.dump();
	elf.dump1();

	while (1) {
	}

	return 0;
}
