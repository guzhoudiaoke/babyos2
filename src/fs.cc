/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#include "fs.h"
#include "babyos.h"
#include "ide.h"
#include "console.h"
#include "string.h"

/******************************************************************************/
void inode_t::init(uint16 major, uint16 minor, uint16 nlink)
{
    m_major = major;
    m_minor = minor;
    m_nlinks = nlink;
    m_size = 0;
    for (int i = 0; i < NDIRECT+1; i++) {
        m_addrs[i] = 0;
    }
}
/******************************************************************************/

void file_system_t::read_super_block()
{
    io_clb_t clb;
    clb.init(m_dev, 1, m_super_block_lba);
    os()->get_ide()->request(&clb);
    memcpy(&m_super_block, clb.buffer, sizeof(super_block_t));
}

void file_system_t::init()
{
    m_dev = 1;
    m_super_block_lba = 1;
    m_inode_lba = 2;

    read_super_block();

    memset(m_inodes, 0, sizeof(inode_t) * MAX_INODE_CACHE);
    memset(m_file_table, 0, sizeof(file_t) * MAX_FILE_NUM);
}

inode_t* file_system_t::get_root()
{
    inode_t* inode = get_inode(ROOT_DEV, ROOT_INUM);
    read_disk_inode(ROOT_INUM, inode);
    return inode;
}

uint32 file_system_t::inode_block(uint32 id)
{
    return m_inode_lba + id / (BSIZE / sizeof(disk_inode_t));
}

uint32 file_system_t::bitmap_block()
{
    return 3 + (m_super_block.m_ninodes * sizeof(disk_inode_t)) / BSIZE;
}

uint32 file_system_t::inode_offset(uint32 id)
{
    return id % (BSIZE / sizeof(disk_inode_t));
}

void file_system_t::read_disk_inode(int id, inode_t* inode)
{
    unsigned block = os()->get_fs()->inode_block(id);
    unsigned offset = os()->get_fs()->inode_offset(id);

    io_clb_t clb;
    clb.init(m_dev, 1, block);
    os()->get_ide()->request(&clb);

    disk_inode_t dinode;
    memcpy(&dinode, clb.buffer + offset*sizeof(disk_inode_t), sizeof(disk_inode_t));
    inode->m_type = dinode.m_type;
    inode->m_major = dinode.m_major;
    inode->m_minor = dinode.m_minor;
    inode->m_nlinks = dinode.m_nlinks;
    inode->m_size = dinode.m_size;
    memcpy(inode->m_addrs, dinode.m_addrs, sizeof(uint32) * (NDIRECT + 1));
}

int file_system_t::read_inode(inode_t* inode, void* dst, uint32 offset, uint32 size)
{
    if (inode->m_type == inode_t::I_TYPE_DEV) {
        dev_op_t* op = os()->get_dev(inode->m_major);
        if (op == NULL) {
            return -1;
        }
        return op->read(inode, dst, size);
    }

    if (offset > inode->m_size) {
        return -1;
    }
    if (offset + size > inode->m_size) {
        size = inode->m_size - offset;
    }

    int nbyte = 0, total = 0;
    while (total < size) {
        io_clb_t clb;
        clb.init(m_dev, 1, block_map(inode, offset / BSIZE));
        os()->get_ide()->request(&clb);
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(dst, clb.buffer + offset % BSIZE, nbyte);
        total += nbyte;
        offset += nbyte;
        dst += nbyte;
    }

    return total;
}

void file_system_t::zero_block(uint32 dev, uint32 b)
{
    io_clb_t clb;
    clb.init(dev, 0, b);
    memset(clb.buffer, 0, BSIZE);
    os()->get_ide()->request(&clb);
}

unsigned file_system_t::alloc_block(uint32 dev)
{
    uint32 index = bitmap_block();
    io_clb_t clb;
    for (unsigned int i = 0; i < m_super_block.m_nblocks; i += BSIZE*8) {
        clb.init(dev, 1, index);
        os()->get_ide()->request(&clb);
        unsigned n = BSIZE*8;
        if (i + BSIZE*8 > m_super_block.m_nblocks) {
            n = m_super_block.m_nblocks - i;
        }
        for (unsigned bit = 0; bit < n; bit++) {
            unsigned flag = 1 << (bit % 8);
            if ((clb.buffer[bit / 8] & flag) == 0) {
                clb.buffer[bit / 8] |= flag;
                clb.init(dev, 0, index);
                os()->get_ide()->request(&clb);
                zero_block(dev, i+bit);
                return i + bit;
            }
        }
        index++;
    }

    return 0;
}

void file_system_t::free_block(uint32 dev, uint32 b)
{
    uint32 index = bitmap_block();
    uint32 block = b / (BSIZE * 8);
    uint32 offset = b % (BSIZE * 8);

    io_clb_t clb;
    clb.init(m_dev, 1, index + block);
    os()->get_ide()->request(&clb);

    uint32 mask = 1 << (offset % 8);
    if ((clb.buffer[offset / 8] & mask) == 0) {
        // need panic
        return;
    }
    clb.buffer[offset / 8] &= ~mask;
    clb.init(m_dev, 0, index + block);
    os()->get_ide()->request(&clb);
}

uint32 file_system_t::block_map(inode_t* inode, uint32 block)
{
    if (block < NDIRECT) {
        if (inode->m_addrs[block] == 0) {
            inode->m_addrs[block] = alloc_block(inode->m_dev);
        }
        return inode->m_addrs[block];
    }

    if (inode->m_addrs[NDIRECT] == 0) {
        inode->m_addrs[NDIRECT] = alloc_block(inode->m_dev);
    }

    io_clb_t clb;
    clb.init(m_dev, 1, inode->m_addrs[NDIRECT]);
    os()->get_ide()->request(&clb);
    
    unsigned* addrs = (unsigned *) clb.buffer;
    uint32 b = addrs[block - NDIRECT];
    if (b == 0) {
        b = addrs[block - NDIRECT] = alloc_block(inode->m_dev);
        clb.init(m_dev, 0, inode->m_addrs[NDIRECT]);
        os()->get_ide()->request(&clb);
    }
    return b;
}

void file_system_t::update_disk_inode(inode_t* inode)
{
    struct disk_inode_t* disk_inode;
    unsigned block = (inode->m_inum / (BSIZE / sizeof(disk_inode_t)) + 2);
    unsigned offset = inode->m_inum % (BSIZE / sizeof(disk_inode_t));

    io_clb_t clb;
    clb.init(m_dev, 1, block);
    os()->get_ide()->request(&clb);

    disk_inode = (disk_inode_t *) clb.buffer + offset;
    disk_inode->m_type = inode->m_type;
    disk_inode->m_major = inode->m_major;
    disk_inode->m_minor = inode->m_minor;
    disk_inode->m_nlinks = inode->m_nlinks;
    disk_inode->m_size = inode->m_size;
    memcpy(disk_inode->m_addrs, inode->m_addrs, sizeof(inode->m_addrs));

    clb.init(m_dev, 0, block);
    os()->get_ide()->request(&clb);
}

inode_t* file_system_t::get_inode(uint32 dev, uint32 inum)
{
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
    if (/*(inode->m_flags & I_VALID) &&*/ inode->m_nlinks == 0) {
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
                io_clb_t clb;
                clb.init(m_dev, 1, inode->m_addrs[NDIRECT]);
                os()->get_ide()->request(&clb);
                uint32* addrs = (uint32 *) clb.buffer;
                for (int i = 0; i < NINDIRECT; i++) {
                    free_block(inode->m_dev, addrs[i]);
                    addrs[i] = 0;
                }
                clb.init(m_dev, 0, inode->m_addrs[NDIRECT]);
                os()->get_ide()->request(&clb);

                free_block(inode->m_dev, inode->m_addrs[NDIRECT]);
                inode->m_addrs[NDIRECT] = 0;
            }

            inode->m_size = 0;
            update_disk_inode(inode);
        }
    }

    inode->m_ref--;
}

inode_t* file_system_t::alloc_inode(uint16 dev, uint16 type)
{
    for (int i = 1; i < m_super_block.m_ninodes; i++) {
        unsigned block = 2 + i / (BSIZE / sizeof(disk_inode_t));
        unsigned offset = i % (BSIZE / sizeof(disk_inode_t));
        io_clb_t clb;
        clb.init(m_dev, 1, block);
        os()->get_ide()->request(&clb);
        disk_inode_t* dinode = ((disk_inode_t *) clb.buffer) + offset;
        if (dinode->m_type == 0) {
            memset(dinode, 0, sizeof(disk_inode_t));
            dinode->m_type = type;
            clb.init(m_dev, 0, block);
            os()->get_ide()->request(&clb);
            return get_inode(dev, i);
        }
    }
    return NULL;
}

inode_t* file_system_t::dir_lookup(inode_t* inode, char* name, unsigned& offset)
{
    dir_entry_t dir;
    unsigned int off = 0;
    while (read_inode(inode, &dir, off, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
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

static const char* skip_elem(const char* path, char* name)
{
    while (*path == '/') {
        path++;
    }
    if (*path == '\0') {
        return NULL;
    }

    const char* begin = path;
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

inode_t* file_system_t::namei(const char* path, int parent, char* name)
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

inode_t* file_system_t::namei(const char* path)
{
    char name[MAX_PATH] = {0};
    return namei(path, 0, name);
}

inode_t* file_system_t::nameiparent(const char* path, char *name)
{
    return namei(path, 1, name);
}

int file_system_t::write_inode(inode_t* inode, void* src, uint32 offset, uint32 size)
{
    if (inode->m_type == inode_t::I_TYPE_DEV) {
        dev_op_t* op = os()->get_dev(inode->m_major);
        if (op == NULL) {
            return -1;
        }
        return op->write(inode, src, size);
    }

    if (offset > inode->m_size) {
        return -1;
    }
    if (offset + size > MAX_FILE_SIZE * BSIZE) {
        return -1;
    }

    int nbyte = 0, total = 0;
    while (total < size) {
        io_clb_t clb;
        clb.init(m_dev, 1, block_map(inode, offset/ BSIZE));
        os()->get_ide()->request(&clb);
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(clb.buffer + offset % BSIZE, src, nbyte);
        clb.init(m_dev, 0, block_map(inode, offset/ BSIZE));
        os()->get_ide()->request(&clb);

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

    write_inode(inode, &dir, offset, sizeof(dir));

    return 0;
}

inode_t* file_system_t::create(const char* path, uint16 type, uint16 major, uint16 minor)
{
    char name[MAX_PATH] = {0};
    inode_t* inode_dir = NULL;
    if ((inode_dir = nameiparent(path, name)) == NULL) {
        return NULL;
    }

    inode_t* inode = NULL;
    unsigned offset = 0;
    if ((inode = dir_lookup(inode_dir, name, offset)) != NULL) {
        if (inode->m_type == inode_t::I_TYPE_FILE && type == inode_t::I_TYPE_FILE) {
            return inode;
        }
        return NULL;
    }

    if ((inode = alloc_inode(inode_dir->m_dev, type)) == NULL) {
        return NULL;
    }

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

    console()->kprintf(YELLOW, "create, %u, %u, { %u, ... }\n", inode->m_type, inode->m_inum, inode->m_addrs[0]);

    put_inode(inode_dir);

    return inode;
}

file_t* file_system_t::alloc_file()
{
    for (int i = 0; i < MAX_FILE_NUM; i++) {
        if (m_file_table[i].m_type == file_t::TYPE_NONE) {
            return &m_file_table[i];
        }
    }
    return NULL;
}

int file_system_t::close_file(file_t* file)
{
    if (file->m_ref < 1) {
        return -1;
    }

    if (--file->m_ref > 0) {
        return 0;
    }

    if (file->m_type == file_t::TYPE_INODE) {
        file->m_type = file_t::TYPE_NONE;
        put_inode(file->m_inode);
    }
}

file_t* file_system_t::dup_file(file_t* file)
{
    file->m_ref++;
    return file;
}

int file_system_t::do_open(const char* path, int mode)
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
    do {
        if (inode->m_type == inode_t::I_TYPE_DIR) {
            break;
        }
        if ((file = alloc_file()) == NULL) {
            break;
        }
        if ((fd = current->alloc_fd(file)) < 0) {
            close_file(file);
            break;
        }
    } while (0);

    if (fd >= 0) {
        file->init(file_t::TYPE_INODE, inode, 0, 
                   !(mode & file_t::MODE_WRONLY), 
                   (mode & file_t::MODE_WRONLY) || (mode & file_t::MODE_RDWR));
    }

    return fd;
}

int file_system_t::do_close(int fd)
{
    file_t* file = current->get_file(fd);
    if (file != NULL) {
        current->close_file(fd);
        close_file(file);
    }

    return 0;
}

int file_system_t::do_read(int fd, void* buffer, uint32 count)
{
    file_t* file = current->get_file(fd);
    if (file == NULL || file->m_readable == 0) {
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

int file_system_t::do_write(int fd, void* buffer, uint32 count)
{
    file_t* file = current->get_file(fd);
    if (file == NULL || file->m_writeable == 0) {
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

int file_system_t::do_mkdir(const char* path)
{
    if (create(path, inode_t::I_TYPE_DIR, 0, 0) == NULL) {
        return -1;
    }
    return 0;
}

int file_system_t::do_link(const char* path_old, const char* path_new)
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
    dir_entry_t de;
    for (uint32 off = 2*sizeof(de); off < inode->m_size; off += sizeof(de)) {
        if (read_inode(inode, &de, off, sizeof(de)) != sizeof(de)) {
            // FIXME: panic
            return false;
        }

        if (de.m_inum != 0) {
            return false;
        }
    }
    return true;
}

int file_system_t::do_unlink(const char* path)
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

int file_system_t::do_mknod(const char* path, int major, int minor)
{
    inode_t* inode = create(path, inode_t::I_TYPE_DEV, major, minor);
    if (inode == NULL) {
        return -1;
    }
    put_inode(inode);
    return 0;
}

int file_system_t::do_dup(int fd)
{
    file_t* file = current->get_file(fd);
    if (file != NULL) {
        int fd = current->alloc_fd(file);
        if (fd < 0) {
            return -1;
        }
        dup_file(file);
        return 0;
    }
    return -1;
}

/******************************************************************************/

void file_system_t::dump_super_block()
{
    console()->kprintf(YELLOW, "super block: \n");
    console()->kprintf(YELLOW, "size: %u block num: %u inode num: %u \n", 
            m_super_block.m_size, m_super_block.m_nblocks, m_super_block.m_ninodes);
}

void file_system_t::test_read_inode()
{
    inode_t inode;
    console()->kprintf(WHITE, "inodes: \n");
    for (int i = 0; i < m_super_block.m_ninodes; i++) {
        read_disk_inode(i, &inode);
        if (inode.m_type != 0) {
            console()->kprintf(WHITE, "%d: { %u, %u, %u, %u, %u }\n", 
                    i, inode.m_type, inode.m_major,
                    inode.m_minor, inode.m_nlinks, inode.m_size);
        }
    }
}

void file_system_t::test_read_bitmap()
{
    uint32 index = bitmap_block();
    console()->kprintf(YELLOW, "bitmap: \n");
    for (unsigned int i = 0; i < m_super_block.m_nblocks; i += BSIZE*8) {
        io_clb_t clb;
        clb.init(m_dev, 1, index);
        os()->get_ide()->request(&clb);
        uint32 size = BSIZE * 8;
        if (size > m_super_block.m_nblocks - i) {
            size = m_super_block.m_nblocks - i;
        }

        uint32* bmp = (uint32 *) clb.buffer;
        for (int j = 0; j < size / 32; j++) {
            console()->kprintf(YELLOW, "%x, ", bmp[j]);
            if (j % 8 == 0) {
                console()->kprintf(YELLOW, "\n");
            }

        }
        index++;
    }
    console()->kprintf(YELLOW, "\n");
}

void file_system_t::test_read_dir_entry()
{
    console()->kprintf(WHITE, "read dir entry: \n");
    inode_t inode;
    for (int i = 0; i < m_super_block.m_ninodes; i++) {
        read_disk_inode(i, &inode);
        if (inode.m_type == inode_t::I_TYPE_DIR) {
            break;
        }
    }

    dir_entry_t dir;
    unsigned int offset = 0;
    while (read_inode(&inode, (char *) &dir, offset, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
        if (dir.m_inum != 0) {
            //console()->kprintf(WHITE, "%s\t %u\n", dir.m_name, dir.m_inum);
            for (int i = 0; i < MAX_PATH; i++) {
                console()->kprintf(WHITE, "%c", dir.m_name[i]);
            }
            console()->kprintf(WHITE, "\t%u\n", dir.m_inum);
        }
        offset += sizeof(dir_entry_t);
    }
}

void file_system_t::test_namei()
{
    console()->kprintf(YELLOW, "test namei: \n");
    inode_t* inode = namei("/ls");
    if (inode != NULL) {
        console()->kprintf(YELLOW, "{ %u, %u, %u, %u, %u }\n", inode->m_type, inode->m_major,
                inode->m_minor, inode->m_nlinks, inode->m_size);
        put_inode(inode);
    }
    else {
        console()->kprintf(YELLOW, "NULL\n");
    }
}

void file_system_t::test_create()
{
    console()->kprintf(YELLOW, "before create file \"abc\" \n");
    test_read_dir_entry();

    int fd = do_open("/abc", file_t::MODE_CREATE);
    do_close(fd);

    console()->kprintf(YELLOW, "after create file \"abc\" \n");
    test_read_dir_entry();
}

void file_system_t::test_read()
{
    console()->kprintf(YELLOW, "test read: \n");
    int fd = do_open((char *) "/test", 0);
    if (fd < 0) {
        return;
    }

    char buffer[512] = {0};
    int n = do_read(fd, buffer, 512);
    console()->kprintf(YELLOW, "%u bytes read from file test: \n%s\n", n, buffer);
    do_close(fd);
}

void file_system_t::test_write()
{
    console()->kprintf(PINK, "test write: \n");
    int fd = do_open("/test", file_t::MODE_RDWR);
    if (fd < 0) {
        return;
    }

    // read
    char buffer[512] = {0};
    int n = do_read(fd, buffer, 512);
    console()->kprintf(WHITE, "%u bytes read from file test: \n%s\n", n, buffer);
    do_close(fd);

    // write
    strcpy(buffer + 37, "2) test write...\n");
    fd = do_open((char *) "/test", file_t::MODE_RDWR);
    n = do_write(fd, buffer, strlen(buffer));
    console()->kprintf(WHITE, "%u bytes write to file\n", n);
    do_close(fd);

    // re-read
    fd = do_open((char *) "/test", file_t::MODE_RDWR);
    memset(buffer, 0, 512);
    n = do_read(fd, buffer, 512);
    console()->kprintf(WHITE, "%u bytes read from file test: \n%s\n", n, buffer);
    do_close(fd);
}

void file_system_t::test_mkdir()
{
    console()->kprintf(PINK, "test mkdir: \n");
    do_mkdir("/test_mkdir");

    int fd = do_open("/test_mkdir/abc", file_t::MODE_CREATE | file_t::MODE_RDWR);
    if (fd < 0) {
        console()->kprintf(PINK, "failed to create abc in dir test_mkdir\n");
        return;
    }

    char buffer[64] = {0};
    strcpy(buffer, "test create file in new maked dir\n");
    int n = do_write(fd, buffer, strlen(buffer));
    console()->kprintf(YELLOW, "%u bytes write to file\n", n);
    do_close(fd);

    fd = do_open((char *) "/test_mkdir/abc", file_t::MODE_RDWR);
    if (fd < 0) {
        console()->kprintf(PINK, "failed to open new file\n");
        return;
    }
    do {
        memset(buffer, 0, 64);
        n = do_read(fd, buffer, 64);
        if (n > 0) {
            console()->kprintf(YELLOW, "%u bytes read from file test: \n%s\n", n, buffer);
        }
    } while (n > 0);
    do_close(fd);

    test_ls("/");
    test_ls("/test_mkdir/");

    return;

}

void file_system_t::test_ls(const char* path)
{
    console()->kprintf(GREEN, "\n%s:\n", path);
    inode_t* inode = namei(path);
    if (inode == NULL) {
        console()->kprintf(RED, "failed to ls %s\n", path);
        return;
    }

    if (inode->m_type == inode_t::I_TYPE_FILE) {
        console()->kprintf(WHITE, "%s %d %d %d %d\n", path, inode->m_type, inode->m_inum, inode->m_size);
    }
    else if (inode->m_type == inode_t::I_TYPE_DIR) {
        dir_entry_t dir;
        unsigned int offset = 0;
        while (read_inode(inode, (char *) &dir, offset, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
            if (dir.m_inum != 0) {
                for (int i = 0; i < MAX_PATH; i++) {
                    console()->kprintf(WHITE, "%c", dir.m_name[i] == '\0' ? ' ' : dir.m_name[i]);
                }
                console()->kprintf(WHITE, "\t%u\n", dir.m_inum);
            }
            offset += sizeof(dir_entry_t);
            memset(dir.m_name, 0, MAX_PATH);
        }
    }

    put_inode(inode);
}

void file_system_t::test_link()
{
    console()->kprintf(PINK, "test link /test to test_mkdir/test.ln: \n");
    do_link("/test", "/test_mkdir/test.ln");
    test_ls("/");
    test_ls("/test_mkdir/");
}

void file_system_t::test_unlink()
{
    console()->kprintf(PINK, "test unlink /test_mkdir/abc: \n");
    do_unlink("/test_mkdir/abc");
    test_ls((char *) "/test_mkdir/");

    console()->kprintf(PINK, "test unlink /test_mkdir: \n");
    if (do_unlink("/test_mkdir") != 0) {
        console()->kprintf(PINK, "failed to unlink /test_mkdir: \n");
    }
    else {
        console()->kprintf(WHITE, "success to unlink /test_mkdir: \n");
    }

    console()->kprintf(PINK, "test unlink /test_mkdir/test.ln: \n");
    do_unlink("/test_mkdir/test.ln");
    test_ls((char *) "/test_mkdir/");

    if (do_unlink("/test_mkdir") != 0) {
        console()->kprintf(PINK, "failed to unlink /test_mkdir: \n");
    }
    else {
        console()->kprintf(WHITE, "success to unlink /test_mkdir: \n");
    }
}

