/*
 * guzhoudiaoke@126.com
 * 2018-01-02
 */

#include "fs.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern process_t* current;
extern file_system_t fs;

void process_t::init()
{
    for (int i = 0; i < MAX_FILE; i++) {
        m_files[i] = NULL;
    }
}

int process_t::alloc_fd(file_t* file)
{
    for (int i = 0; i < MAX_FILE; i++) {
        if (m_files[i] == NULL) {
            m_files[i] = file;
            return i;
        }
    }
    return -1;
}

/************************************************************************/

void inode_t::init(uint16 major, uint16 minor, uint16 nlink)
{
    m_major = major;
    m_minor = minor;
    m_nlinks = nlink;
    for (int i = 0; i < NDIRECT+1; i++) {
        m_addrs[i] = 0;
    }
}

void inode_t::lock()
{
}

void inode_t::unlock()
{
}


/************************************************************************/

void file_system_t::read_block(unsigned block_index, unsigned char* buffer)
{
    FILE* fp = fopen("fs.img", "rb");
    if (fp != NULL) {
        fseek(fp, BSIZE*block_index, SEEK_SET);
        fread(buffer, 1, BSIZE, fp);
        fclose(fp);
    }
}

void file_system_t::write_block(unsigned block_index, unsigned char* buffer)
{
    //printf("write block %u\n", block_index);
    FILE* fp = fopen("fs.img", "rb+");
    if (fp != NULL) {
        fseek(fp, BSIZE*block_index, SEEK_SET);
        fwrite(buffer, 1, BSIZE, fp);
        fclose(fp);
    }
}

void file_system_t::zero_block(uint32 dev, uint32 b)
{
    unsigned char buffer[512] = {0};
    memset(buffer, 0, 512);
    write_block(b, buffer);
}

unsigned file_system_t::alloc_block(unsigned dev)
{
    unsigned char buffer[BSIZE] = {0};
    unsigned int index = 3 + (m_super_block.m_ninodes * sizeof(disk_inode_t)) / BSIZE;
    //unsigned int index = 2 + (m_super_block.m_ninodes * sizeof(disk_inode_t) + BSIZE - 1) / BSIZE;
    for (unsigned int i = 0; i < m_super_block.m_nblocks; i += BSIZE*8) {
        read_block(index, buffer);
        unsigned n = BSIZE*8;
        if (i + BSIZE*8 > m_super_block.m_nblocks) {
            n = m_super_block.m_nblocks - i;
        }
        for (unsigned bit = 0; bit < n; bit++) {
            unsigned flag = 1 << (bit % 8);
            if ((buffer[bit / 8] & flag) == 0) {
                buffer[bit / 8] |= flag;
                write_block(index, buffer);
                zero_block(dev, i+bit);
                return i + bit;
            }
        }
        index++;
    }

    return 0;
}

void file_system_t::free_block(unsigned dev, unsigned b)
{
    //printf("free block : %u\n", b);
    unsigned int index = 3 + (m_super_block.m_ninodes * sizeof(disk_inode_t)) / BSIZE;
    //unsigned int index = 2 + (m_super_block.m_ninodes * sizeof(disk_inode_t) + BSIZE - 1) / BSIZE;
    unsigned block = index + b / (BSIZE * 8);
    unsigned offset = b % (BSIZE * 8);

    unsigned char buffer[512];
    read_block(block, buffer);

    uint32 mask = 1 << (offset % 8);
    if ((buffer[offset / 8] & mask) == 0) {
        // need panic
        return;
    }
    buffer[offset / 8] &= ~mask;
    write_block(block, buffer);
}

void file_system_t::read_super_block()
{
    unsigned char buffer[512];
    read_block(1, buffer);
    memcpy(&m_super_block, buffer, sizeof(super_block_t));
}

void file_system_t::init()
{
    read_super_block();
    m_block_bmp.init(m_super_block.m_nblocks);

    unsigned char buffer[BSIZE] = {0};
    unsigned int index = 3 + (m_super_block.m_ninodes * sizeof(disk_inode_t)) / BSIZE;
    //unsigned int index = 2 + (m_super_block.m_ninodes * sizeof(disk_inode_t) + BSIZE - 1) / BSIZE;
    for (unsigned int i = 0; i < m_super_block.m_nblocks; i += BSIZE*8) {
        read_block(index, buffer);
        unsigned n = BSIZE*8;
        if (i + BSIZE*8 > m_super_block.m_nblocks) {
            n = m_super_block.m_nblocks - i;
        }
        m_block_bmp.setbits(buffer, i, n);
        index++;
    }

    inode_t* inode = get_inode(ROOT_DEV, ROOT_INUM);
    read_disk_inode(ROOT_INUM, inode);

    current->m_cwd = inode;
}

inode_t* file_system_t::create(char* path, uint16 type, uint16 major, uint16 minor)
{
    char name[MAX_PATH] = {0};
    inode_t* inode_dir = NULL;
    if ((inode_dir = nameiparent(path, name)) == NULL) {
        return NULL;
    }

    inode_t* inode = NULL;
    inode_dir->lock();

    unsigned offset = 0;
    if ((inode = dir_lookup(inode_dir, name, offset)) != NULL) {
        inode_dir->unlock();

        inode->lock();
        if (inode->m_type == inode_t::I_TYPE_FILE && type == inode_t::I_TYPE_FILE) {
            inode->unlock();
            return inode;
        }
        inode->unlock();
        return NULL;
    }

    if ((inode = alloc_inode(inode_dir->m_dev, type)) == NULL) {
        inode_dir->unlock();
        return NULL;
    }

    inode->lock();
    inode->init(major, minor, 1);
    inode->m_type = type;
    update_disk_inode(inode);

    if (inode->m_type == inode_t::I_TYPE_DIR) {
        inode_dir->m_nlinks++;
        update_disk_inode(inode_dir);
        if (dir_link(inode, (char *) ".", inode->m_inum) < 0 || 
            dir_link(inode, (char *) "..", inode->m_inum) < 0) {
            return NULL;
        }
    }

    dir_link(inode_dir, name, inode->m_inum);
    inode->unlock();

    inode_dir->unlock();

    return inode;
}

int file_system_t::do_open(char* path, int mode)
{
    inode_t* inode = NULL;
    file_t* file = NULL;

    if (mode & file_t::MODE_CREATE) {
        if ((inode = create(path, inode_t::I_TYPE_FILE, 0, 0)) == NULL) {
            return -1;
        }
    }
    else {
        if ((inode = namei(path)) == NULL) {
            return -1;
        }
    }

    int fd = -1;
    inode->lock();
    do {
        if (inode->m_type == inode_t::I_TYPE_DIR) {
            break;
        }
        if ((file = alloc_file()) == NULL) {
            break;
        }
        if ((fd = current->alloc_fd(file)) < 0) {
            file->close();
            break;
        }
    } while (0);
    inode->unlock();

    if (fd >= 0) {
        file->init(file_t::TYPE_INODE, inode, 0, 
                   !(mode & file_t::MODE_WRONLY), 
                   (mode & file_t::MODE_WRONLY) || (mode & file_t::MODE_RDWR));
    }

    return fd;
}

int file_system_t::do_close(int fd)
{
    if (fd < 0 || fd >= MAX_FILE) {
        return -1;
    }

    file_t* file = current->m_files[fd];
    current->m_files[fd] = NULL;

    return 0;
}

int file_system_t::do_read(int fd, void* buffer, unsigned count)
{
    if (fd < 0 || fd >= MAX_FILE) {
        return -1;
    }

    file_t* file = current->m_files[fd];
    if (file->m_readable == 0) {
        return -1;
    }

    if (file->m_type == file_t::TYPE_INODE) {
        int nbyte = 0;
        if ((nbyte = read_inode(file->m_inode, (char *) buffer, file->m_offset, count)) > 0) {
            file->m_offset += nbyte;
        }
        return nbyte;
    }

    return -1;
}

int file_system_t::do_write(int fd, void* buffer, unsigned count)
{
    if (fd < 0 || fd >= MAX_FILE) {
        return -1;
    }

    file_t* file = current->m_files[fd];
    if (file->m_writeable == 0) {
        return -1;
    }

    if (file->m_type == file_t::TYPE_INODE) {
        int nbyte = 0;
        if ((nbyte = write_inode(file->m_inode, (char *) buffer, file->m_offset, count)) > 0) {
            file->m_offset += nbyte;
        }
        return nbyte;
    }

    return -1;
}

int file_system_t::do_mkdir(char* path)
{
    if (create(path, inode_t::I_TYPE_DIR, 0, 0) == NULL) {
        return -1;
    }
    return 0;
}

int file_system_t::do_link(char* path_old, char* path_new)
{
    inode_t* inode = NULL;
    if ((inode = namei(path_old)) == NULL) {
        return -1;
    }

    if (inode->m_type == inode_t::I_TYPE_DIR) {
        return -1;
    }

    inode_t* dir = NULL;
    char name[MAX_PATH] = {0};
    if ((dir = nameiparent(path_new, name)) == NULL) {
        return -1;
    }

    if (dir->m_dev != inode->m_dev) {
        return -1;
    }
    if (dir_link(dir, name, inode->m_inum) < 0) {
        return -1;
    }

    inode->m_nlinks++;
    update_disk_inode(inode);

    put_inode(dir);
    put_inode(inode);

    return 0;
}

bool file_system_t::dir_empty(inode_t* inode)
{
    return false;
}

int file_system_t::do_unlink(char* path)
{
    inode_t* dir = NULL;
    char name[MAX_PATH] = {0};
    if ((dir = nameiparent(path, name)) == NULL) {
        return -1;
    }

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        put_inode(dir);
        return -1;
    }

    inode_t* inode = NULL;
    unsigned offset = 0;
    if ((inode = dir_lookup(dir, name, offset)) == NULL) {
        put_inode(dir);
        return -1;
    }

    if (inode->m_nlinks < 1) {
        return -1;
    }

    if (inode->m_type == inode_t::I_TYPE_DIR && !dir_empty(inode)) {
        put_inode(dir);
        put_inode(inode);
        return -1;
    }

    dir_entry_t de;
    memset(&de, 0, sizeof(de));
    write_inode(dir, (char *) &de, offset, sizeof(de));

    if (inode->m_type == inode_t::I_TYPE_DIR) {
        dir->m_nlinks--;
        update_disk_inode(dir);
    }
    put_inode(dir);

    inode->m_nlinks--;
    update_disk_inode(inode);
    put_inode(inode);

    return 0;
}

file_t* file_system_t::alloc_file()
{
    return (file_t *) malloc(sizeof(file_t));
}

inode_t* file_system_t::get_inode(uint32 dev, uint32 inum)
{
    // FIXME: need a lock
    inode_t* inode = NULL;
    inode_t* empty = NULL;
    for (int i = 0; i < MAX_INODE_CACHE; i++) {
        if (m_inodes[i].m_ref > 0 && m_inodes[i].m_dev == dev && m_inodes[i].m_inum == inum) {
            inode = &m_inodes[i];
            inode->m_ref++;
            return inode;
        }
        if (empty == NULL && m_inodes[i].m_ref == 0) {
            empty = &m_inodes[i]; }
    } 
    if (empty != NULL) {
        inode = empty;
        inode->m_dev = dev;
        inode->m_inum = inum;
        inode->m_ref = 1;
        inode->m_flags = 0;
    }
    return inode;
}

inode_t* file_system_t::dup_inode(inode_t* inode)
{
    inode->m_ref++;
    return inode;
}

inode_t* file_system_t::put_inode(inode_t* inode)
{
    if (/*(inode->m_flags & I_VALID) && */inode->m_nlinks == 0) {
        if (inode->m_ref == 1) {
            inode->m_flags = 0;
            inode->m_type = 0;

            for (int i = 0; i < NDIRECT; i++) {
                if (inode->m_addrs[i] != 0) {
                    free_block(inode->m_dev, inode->m_addrs[i]);
                    inode->m_addrs[i] = 0;
                }
            }

            if (inode->m_addrs[NDIRECT] != 0) {
                unsigned char buffer[512];
                read_block(inode->m_addrs[NDIRECT], buffer);
                uint32* addrs = (uint32 *) buffer;
                for (int i = 0; i < NINDIRECT; i++) {
                    if (addrs[i] != 0) {
                        free_block(inode->m_dev, addrs[i]);
                        addrs[i] = 0;
                    }
                }
                write_block(inode->m_addrs[NDIRECT], buffer);

                free_block(inode->m_dev, inode->m_addrs[NDIRECT]);
                inode->m_addrs[NDIRECT] = 0;
            }

            inode->m_size = 0;
            update_disk_inode(inode);
        }
    }

    inode->m_ref--;
}

static char* skip_elem(char* path, char* name)
{
    while (*path == '/') {
        path++;
    }
    if (*path == '\0') {
        return NULL;
    }

    char* begin = path;
    while (*path != '/' && *path != '\0') {
        path++;
    }

    int len = path - begin;
    if (len >= MAX_PATH) {
        len = MAX_PATH;
    }

    memset(name, 0, MAX_PATH);
    strncpy(name, begin, len);
    while (*path == '/') {
        path++;
    }

    return path;
}

inode_t* file_system_t::namei(char* path, int parent, char* name)
{
    inode_t* inode = NULL;
    inode_t* next = NULL;

    if (*path == '/') {
        inode = get_inode(ROOT_DEV, ROOT_INUM);
    }
    else {
        inode = dup_inode(current->m_cwd);
    }

    while ((path = skip_elem(path, name)) != NULL) {
        if (inode->m_type != inode_t::I_TYPE_DIR) {
            return NULL;
        }

        if (parent && *path == '\0') {
            return inode;
        }

        unsigned offset = 0;
        if ((next = dir_lookup(inode, name, offset)) == NULL) {
            return NULL;
        }

        put_inode(inode);
        inode = next;
    }

    if (parent) {
        put_inode(inode);
        return NULL;
    }

    return inode;
}

inode_t* file_system_t::namei(char* path)
{
    char name[MAX_PATH] = {0};
    return namei(path, 0, name);
}

inode_t* file_system_t::nameiparent(char* path, char *name)
{
    return namei(path, 1, name);
}

int file_system_t::dir_link(inode_t* inode, char* name, uint32 inum)
{
    // already present in the dir
    inode_t* find = NULL;
    unsigned offset = 0;
    if ((find = dir_lookup(inode, name, offset)) != NULL) {
        put_inode(inode);
        return -1;
    }

    // find an empty dir_entry
    dir_entry_t dir;
    for (offset = 0; offset < inode->m_size; offset += sizeof(dir)) {
        if (read_inode(inode, (char *) &dir, offset, sizeof(dir)) != sizeof(dir)) {
            return -1;
        }
        if (dir.m_inum == 0) {
            break;
        }
    }

    dir.m_inum = inum;
    memset(dir.m_name, 0, MAX_PATH);
    strncpy(dir.m_name, name, MAX_PATH);

    write_inode(inode, (char *) &dir, offset, sizeof(dir));

    return 0;
}

void file_system_t::update_disk_inode(inode_t* inode)
{
    struct disk_inode_t* disk_inode;
    unsigned char buffer[512];
    unsigned block = (inode->m_inum / (BSIZE / sizeof(disk_inode_t)) + 2);
    unsigned offset = inode->m_inum % (BSIZE / sizeof(disk_inode_t));

    read_block(block, buffer);
    disk_inode = (disk_inode_t *) buffer + offset;

    disk_inode->m_type = inode->m_type;
    disk_inode->m_major = inode->m_major;
    disk_inode->m_minor = inode->m_minor;
    disk_inode->m_nlinks = inode->m_nlinks;
    disk_inode->m_size = inode->m_size;
    memcpy(disk_inode->m_addrs, inode->m_addrs, sizeof(inode->m_addrs));

    write_block(block, buffer);
}

inode_t* file_system_t::alloc_inode(uint16 dev, uint16 type)
{
    unsigned char buffer[512];
    for (int i = 1; i < fs.m_super_block.m_ninodes; i++) {
        unsigned block = 2 + i / (BSIZE / sizeof(disk_inode_t));
        unsigned offset = i % (BSIZE / sizeof(disk_inode_t));
        read_block(block, buffer);
        disk_inode_t* dinode = ((disk_inode_t *) buffer) + offset;
        if (dinode->m_type == 0) {
            memset(dinode, 0, sizeof(disk_inode_t));
            dinode->m_type = type;
            write_block(block, buffer);
            return get_inode(dev, i);
        }
    }
    return NULL;
}

inode_t* file_system_t::dir_lookup(inode_t* inode, char* name, unsigned& offset)
{
    dir_entry_t dir;
    unsigned int off = 0;
    while (true) {
        if (read_inode(inode, (char *) &dir, off, sizeof(dir_entry_t)) != sizeof(dir_entry_t)) {
            break;
        }

        if (dir.m_inum != 0) {
            if (strcmp(name, dir.m_name) == 0) {
                unsigned inum = dir.m_inum;
                inode = get_inode(inode->m_dev, inum);
                read_disk_inode(inum, inode);
                offset = off;
                return inode;
            }
        }

        off += sizeof(dir_entry_t);
    }
    return NULL;
}

void file_system_t::read_disk_inode(unsigned index, inode_t* inode)
{
    unsigned block = 2 + index / (BSIZE / sizeof(disk_inode_t));
    unsigned offset = index % (BSIZE / sizeof(disk_inode_t));

    unsigned char buffer[512];
    read_block(block, buffer);

    disk_inode_t dinode;
    memcpy(&dinode, buffer + offset*sizeof(disk_inode_t), sizeof(disk_inode_t));
    inode->m_type = dinode.m_type;
    inode->m_major = dinode.m_major;
    inode->m_minor = dinode.m_minor;
    inode->m_nlinks = dinode.m_nlinks;
    inode->m_size = dinode.m_size;
    for (int i = 0; i < NDIRECT+1; i++) {
        inode->m_addrs[i] = dinode.m_addrs[i];
    }
}

int file_system_t::read_inode(inode_t* inode, char* dst, uint32 offset, uint32 size)
{
    if (offset > inode->m_size) {
        return -1;
    }
    if (offset + size > inode->m_size) {
        size = inode->m_size - offset;
    }
    if (size == 0) {
        return 0;
    }

    int nbyte = 0, total = 0;
    unsigned char buffer[512];
    while (total < size) {
        read_block(block_map(inode, offset / BSIZE), buffer);
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(dst, buffer + offset % BSIZE, nbyte);
        total += nbyte;
        offset += nbyte;
        dst += nbyte;
    }

    return total;
}

int file_system_t::write_inode(inode_t* inode, char* src, uint32 offset, uint32 size)
{
    if (offset > inode->m_size) {
        return -1;
    }
    if (offset + size > MAXFILE * BSIZE) {
        return -1;
    }

    int nbyte = 0, total = 0;
    unsigned char buffer[512] = {0};
    while (total < size) {
        read_block(block_map(inode, offset / BSIZE), buffer);
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(buffer + offset % BSIZE, src, nbyte);
        //printf("write inode offset: %u\n", offset);
        write_block(block_map(inode, offset / BSIZE), buffer);

        total += nbyte;
        offset += nbyte;
        src += nbyte;
    }

    if (total > 0 && offset > inode->m_size) {
        inode->m_size = offset;
        update_disk_inode(inode);
    }

    return total;
}


uint32 file_system_t::block_map(inode_t* inode, uint32 block)
{
    if (block < NDIRECT) {
        if (inode->m_addrs[block] == 0) {
            inode->m_addrs[block] = alloc_block(inode->m_dev);
        }
        return inode->m_addrs[block];
    }

    unsigned char buffer[512] = {0};
    if (inode->m_addrs[NDIRECT] == 0) {
        inode->m_addrs[NDIRECT] = alloc_block(inode->m_dev);
    }
    read_block(inode->m_addrs[NDIRECT], buffer);
    
    unsigned* addrs = (unsigned *) buffer;
    unsigned b = addrs[block - NDIRECT];
    if (b == 0) {
        b = addrs[block - NDIRECT] = alloc_block(inode->m_dev);
        write_block(inode->m_addrs[NDIRECT], buffer);
    }

    //printf("block map: %u->%u\n", block, b);
    return b;
}

