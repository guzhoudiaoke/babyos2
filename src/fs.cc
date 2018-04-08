/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#include "fs.h"
#include "babyos.h"
#include "console.h"
#include "string.h"
#include "block_dev.h"
#include "socket.h"


void file_system_t::read_super_block(super_block_t* sb)
{
    io_buffer_t* b = os()->get_block_dev()->read(m_super_block_lba);
    memcpy(sb, b->m_buffer, sizeof(super_block_t));
    os()->get_block_dev()->release_block(b);
}

void file_system_t::init()
{
    m_dev = 1;
    m_super_block_lba = 1;
    m_inode_lba = 2;

    m_inodes_lock.init();
    m_file_table.init();

    memset(m_inodes, 0, sizeof(inode_t) * MAX_INODE_CACHE);
}

inode_t* file_system_t::get_root()
{
    inode_t* inode = get_inode(ROOT_DEV, ROOT_INUM);
    inode->read_from_disk(ROOT_INUM);
    return inode;
}

uint32 file_system_t::inode_block(uint32 id)
{
    return m_inode_lba + id / (BSIZE / sizeof(disk_inode_t));
}

uint32 file_system_t::bitmap_block()
{
    super_block_t sb;
    read_super_block(&sb);
    return 3 + (sb.m_ninodes * sizeof(disk_inode_t)) / BSIZE;
}

uint32 file_system_t::inode_offset(uint32 id)
{
    return id % (BSIZE / sizeof(disk_inode_t));
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
        io_buffer_t* b = os()->get_block_dev()->read(block_map(inode, offset / BSIZE));
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(dst, b->m_buffer + offset % BSIZE, nbyte);
        os()->get_block_dev()->release_block(b);

        total += nbyte;
        offset += nbyte;
        dst += nbyte;
    }

    return total;
}

void file_system_t::zero_block(uint32 dev, uint32 block)
{
    io_buffer_t* b = os()->get_block_dev()->read(block);
    memset(b->m_buffer, 0, BSIZE);
    os()->get_block_dev()->write(b);
    os()->get_block_dev()->release_block(b);
}

unsigned file_system_t::alloc_block(uint32 dev)
{
    super_block_t sb;
    read_super_block(&sb);
    uint32 index = bitmap_block();
    for (unsigned int i = 0; i < sb.m_nblocks; i += BSIZE*8) {
        io_buffer_t* b = os()->get_block_dev()->read(index);
        unsigned n = BSIZE*8;
        if (i + BSIZE*8 > sb.m_nblocks) {
            n = sb.m_nblocks - i;
        }
        for (unsigned bit = 0; bit < n; bit++) {
            unsigned flag = 1 << (bit % 8);
            if ((b->m_buffer[bit / 8] & flag) == 0) {
                b->m_buffer[bit / 8] |= flag;
                os()->get_block_dev()->write(b);
                os()->get_block_dev()->release_block(b);
                zero_block(dev, i+bit);
                return i + bit;
            }
        }
        os()->get_block_dev()->release_block(b);
        index++;
    }

    return 0;
}

void file_system_t::free_block(uint32 dev, uint32 lba)
{
    uint32 index = bitmap_block();
    uint32 block = lba / (BSIZE * 8);
    uint32 offset = lba % (BSIZE * 8);

    io_buffer_t* b = os()->get_block_dev()->read(index+block);
    uint32 mask = 1 << (offset % 8);
    if ((b->m_buffer[offset / 8] & mask) == 0) {
        // need panic
        return;
    }
    b->m_buffer[offset / 8] &= ~mask;
    os()->get_block_dev()->write(b);
    os()->get_block_dev()->release_block(b);
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

    io_buffer_t* b = os()->get_block_dev()->read(inode->m_addrs[NDIRECT]);
    
    unsigned* addrs = (unsigned *) b->m_buffer;
    uint32 lba = addrs[block - NDIRECT];
    if (lba == 0) {
        lba = addrs[block - NDIRECT] = alloc_block(inode->m_dev);
        os()->get_block_dev()->write(b);
    }
    os()->get_block_dev()->release_block(b);
    return lba;
}

inode_t* file_system_t::get_inode(uint32 dev, uint32 inum)
{
    inode_t* inode = NULL;
    inode_t* empty = NULL;

    uint32 flags;
    m_inodes_lock.lock_irqsave(flags);
    for (int i = 0; i < MAX_INODE_CACHE; i++) {
        if (m_inodes[i].m_ref > 0 && m_inodes[i].m_dev == dev && m_inodes[i].m_inum == inum) {
            inode = &m_inodes[i];
            inode->m_ref++;
            m_inodes_lock.unlock_irqrestore(flags);
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
        inode->m_valid = 0;
        inode->m_sem.init(1);

        inode->m_size = 0;
        for (int i = 0; i < NDIRECT+1; i++) {
            inode->m_addrs[i] = 0;
        }
    }
    m_inodes_lock.unlock_irqrestore(flags);

    return inode;
}

inode_t* file_system_t::dup_inode(inode_t* inode)
{
    locker_t locker(m_inodes_lock);
    inode->m_ref++;

    return inode;
}

void file_system_t::put_inode(inode_t* inode)
{
    inode->lock();
    if (inode->m_valid && inode->m_nlinks == 0) {
        if (inode->m_ref == 1) {
            inode->m_type = 0;

            for (int i = 0; i < NDIRECT; i++) {
                if (inode->m_addrs[i] != 0) {
                    free_block(inode->m_dev, inode->m_addrs[i]);
                    inode->m_addrs[i] = 0;
                }
            }

            if (inode->m_addrs[NDIRECT] != 0) {
                io_buffer_t* b = os()->get_block_dev()->read(inode->m_addrs[NDIRECT]);
                uint32* addrs = (uint32 *) b->m_buffer;
                for (int i = 0; i < NINDIRECT; i++) {
                    free_block(inode->m_dev, addrs[i]);
                    addrs[i] = 0;
                }
                os()->get_block_dev()->write(b);
                os()->get_block_dev()->release_block(b);

                free_block(inode->m_dev, inode->m_addrs[NDIRECT]);
                inode->m_addrs[NDIRECT] = 0;
            }

            inode->m_size = 0;
            inode->write_to_disk();

            inode->m_valid = 0;
        }
    }
    inode->unlock();

    uint32 flags;
    m_inodes_lock.lock_irqsave(flags);
    inode->m_ref--;
    m_inodes_lock.unlock_irqrestore(flags);
}

inode_t* file_system_t::alloc_inode(uint16 dev, uint16 type)
{
    super_block_t sb;
    read_super_block(&sb);
    for (int i = 1; i < sb.m_ninodes; i++) {
        unsigned block = 2 + i / (BSIZE / sizeof(disk_inode_t));
        unsigned offset = i % (BSIZE / sizeof(disk_inode_t));
        io_buffer_t* b = os()->get_block_dev()->read(block);
        disk_inode_t* dinode = ((disk_inode_t *) b->m_buffer) + offset;
        if (dinode->m_type == 0) {
            memset(dinode, 0, sizeof(disk_inode_t));
            dinode->m_type = type;
            os()->get_block_dev()->write(b);
            os()->get_block_dev()->release_block(b);
            return get_inode(dev, i);
        }
        os()->get_block_dev()->release_block(b);
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
                inode->read_from_disk(inum);
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
        inode->lock();
        if (inode->m_type != inode_t::I_TYPE_DIR) {
            inode->unlock();
            put_inode(inode);
            return NULL;
        }

        if (parent && *path == '\0') {
            inode->unlock();
            return inode;
        }

        unsigned offset = 0;
        if ((next = dir_lookup(inode, name, offset)) == NULL) {
            inode->unlock();
            put_inode(inode);
            return NULL;
        }

        inode->unlock();
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
        io_buffer_t* b = os()->get_block_dev()->read(block_map(inode, offset / BSIZE));
        nbyte = BSIZE - offset % BSIZE;
        if (nbyte > size - total) {
            nbyte = size - total;
        }

        memcpy(b->m_buffer + offset % BSIZE, src, nbyte);
        os()->get_block_dev()->write(b);
        os()->get_block_dev()->release_block(b);

        total += nbyte;
        offset += nbyte;
        src += nbyte;
    }

    if (total > 0 && offset > inode->m_size) {
        inode->m_size = offset;
        inode->write_to_disk();
    }

    return total;
}

int file_system_t::dir_link(inode_t* inode, char* name, uint32 inum)
{
    /* already present in the dir */
    inode_t* find = NULL;
    unsigned offset = 0;
    if ((find = dir_lookup(inode, name, offset)) != NULL) {
        put_inode(inode);
        return -1;
    }

    /* find an empty dir_entry */
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
    inode_dir->lock();

    inode_t* inode = NULL;
    unsigned offset = 0;
    if ((inode = dir_lookup(inode_dir, name, offset)) != NULL) {
        inode_dir->unlock();
        put_inode(inode_dir);

        inode->lock();
        if (inode->m_type == inode_t::I_TYPE_FILE && type == inode_t::I_TYPE_FILE) {
            return inode;
        }
        inode->unlock();
        put_inode(inode);
        return NULL;
    }

    if ((inode = alloc_inode(inode_dir->m_dev, type)) == NULL) {
        inode_dir->unlock();
        return NULL;
    }

    inode->lock();
    inode->m_major = major;
    inode->m_minor = minor;
    inode->m_nlinks = 1;
    inode->m_type = type;
    inode->write_to_disk();

    if (inode->m_type == inode_t::I_TYPE_DIR) {
        inode_dir->m_nlinks++;
        inode_dir->write_to_disk();
        if (dir_link(inode, (char *) ".", inode->m_inum) < 0 || 
            dir_link(inode, (char *) "..", inode->m_inum) < 0) {
            inode->unlock();
            inode_dir->unlock();
            return NULL;
        }
    }

    dir_link(inode_dir, name, inode->m_inum);

    inode_dir->unlock();
    put_inode(inode_dir);

    return inode;
}

file_t* file_system_t::alloc_file()
{
    return m_file_table.alloc();
}

int file_system_t::close_file(file_t* file)
{
    return m_file_table.free(file);
}

file_t* file_system_t::dup_file(file_t* file)
{
    return m_file_table.dup_file(file);
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
        if (inode->m_type == inode_t::I_TYPE_DIR && mode != file_t::MODE_RDONLY) {
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

    inode->unlock();
    if (fd >= 0) {
        file->init(file_t::TYPE_INODE, inode, NULL, 0, 
                   !(mode & file_t::MODE_WRONLY), 
                   (mode & file_t::MODE_WRONLY) || (mode & file_t::MODE_RDWR));
    }

    return fd;
}

int file_system_t::do_close(int fd)
{
    file_t* file = current->get_file(fd);
    if (file != NULL) {
        current->free_fd(fd);
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

    if (file->m_type == file_t::TYPE_PIPE) {
        return file->m_pipe->read(buffer, count);
    }
    if (file->m_type == file_t::TYPE_SOCKET) {
        return file->m_socket->read(buffer, count);
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

    if (file->m_type == file_t::TYPE_PIPE) {
        return file->m_pipe->write(buffer, count);
    }
    if (file->m_type == file_t::TYPE_SOCKET) {
        return file->m_socket->write(buffer, count);
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
    inode_t* inode = NULL;
    if ((inode = create(path, inode_t::I_TYPE_DIR, 0, 0)) == NULL) {
        return -1;
    }
    inode->unlock();
    put_inode(inode);
    return 0;
}

int file_system_t::do_link(const char* path_old, const char* path_new)
{
    inode_t* inode = NULL;
    inode_t* dir = NULL;

    if ((inode = namei(path_old)) == NULL) {
        return -1;
    }

    inode->lock();
    if (inode->m_type == inode_t::I_TYPE_DIR) {
        inode->unlock();
        put_inode(inode);
        return -1;
    }

    inode->m_nlinks++;
    inode->write_to_disk();
    inode->unlock();

    char name[MAX_PATH] = {0};
    if ((dir = nameiparent(path_new, name)) == NULL) {
        goto failed;
    }

    dir->lock();
    if (dir->m_dev != inode->m_dev || dir_link(dir, name, inode->m_inum) < 0) {
        dir->unlock();
        put_inode(dir);
        goto failed;
    }

    dir->unlock();
    put_inode(dir);
    put_inode(inode);

    return 0;

failed:
    inode->lock();
    inode->m_nlinks--;
    inode->write_to_disk();
    inode->unlock();

    return -1;
}

bool file_system_t::dir_empty(inode_t* inode)
{  
    dir_entry_t de;
    for (uint32 off = 2*sizeof(de); off < inode->m_size; off += sizeof(de)) {
        if (read_inode(inode, &de, off, sizeof(de)) != sizeof(de)) {
            os()->panic("failed to read inode");
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
    inode_t* inode = NULL;
    dir_entry_t de;

    char name[MAX_PATH] = {0};
    unsigned offset = 0;

    if ((dir = nameiparent(path, name)) == NULL) {
        return -1;
    }

    dir->lock();
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        goto failed;
    }

    if ((inode = dir_lookup(dir, name, offset)) == NULL) {
        goto failed;
    }

    inode->lock();
    if (inode->m_nlinks < 1) {
        goto failed;
    }

    if (inode->m_type == inode_t::I_TYPE_DIR && !dir_empty(inode)) {
        inode->unlock();
        put_inode(inode);
        goto failed;
    }

    memset(&de, 0, sizeof(de));
    write_inode(dir, (char *) &de, offset, sizeof(de));

    if (inode->m_type == inode_t::I_TYPE_DIR) {
        dir->m_nlinks--;
        dir->write_to_disk();
    }
    dir->unlock();
    put_inode(dir);

    inode->m_nlinks--;
    inode->write_to_disk();
    inode->unlock();
    put_inode(inode);

    return 0;

failed:
    dir->unlock();
    put_inode(dir);
    return -1;
}

int file_system_t::do_mknod(const char* path, int major, int minor)
{
    inode_t* inode = create(path, inode_t::I_TYPE_DEV, major, minor);
    if (inode == NULL) {
        return -1;
    }
    inode->unlock();
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

int file_system_t::do_stat(int fd, stat_t* st)
{
    file_t* file = current->get_file(fd);
    if (file != NULL && file->m_inode != NULL) {
        st->m_type      = file->m_inode->m_type;
        st->m_nlinks    = file->m_inode->m_nlinks;
        st->m_dev       = file->m_inode->m_dev;
        st->m_size      = file->m_inode->m_size;
        return 0;
    }
    return -1;
}

int file_system_t::do_seek(int fd, uint32 pos)
{
    file_t* file = current->get_file(fd);
    if (file != NULL && file->m_inode != NULL && pos < file->m_inode->m_size) {
        file->m_offset = pos;
        return 0;
    }
    return -1;
}

int file_system_t::do_chdir(const char* path)
{
    inode_t* inode = namei(path);
    if (inode == NULL) {
        return -1;
    }
    if (inode->m_type != inode_t::I_TYPE_DIR) {
        inode->unlock();
        put_inode(inode);
        return -1;
    }

    inode->unlock();
    put_inode(current->m_cwd);
    current->m_cwd = inode;

    return 0;
}

int file_system_t::alloc_pipe(file_t*& file_read, file_t*& file_write)
{
    pipe_t* pipe = NULL;

    file_read = alloc_file();
    if (file_read == NULL) {
        goto failed;
    }
    file_write = alloc_file();
    if (file_write == NULL) {
        goto failed;
    }

    pipe = (pipe_t *) os()->get_obj_pool(PIPE_POOL)->alloc_from_pool();
    if (pipe == NULL) {
        goto failed;
    }

    pipe->init();
    file_read->init(file_t::TYPE_PIPE, NULL, pipe, 0, 1, 0);
    file_write->init(file_t::TYPE_PIPE, NULL, pipe, 0, 0, 1);

    return 0;

failed:
    if (file_read != NULL) {
        close_file(file_read);
    }
    if (file_write != NULL) {
        close_file(file_write);
    }
    if (pipe != NULL) {
        os()->get_obj_pool(PIPE_POOL)->free_object((void *) pipe);
    }
    return -1;
}

int file_system_t::do_pipe(int fd[2])
{
    file_t* file_read = NULL;
    file_t* file_write = NULL;
    int fd_read = -1, fd_write = -1;
    if (alloc_pipe(file_read, file_write) < 0) {
        return -1;
    }

    fd_read = current->alloc_fd(file_read);
    if (fd_read < 0) {
        goto failed;
    }
    fd_write = current->alloc_fd(file_write);
    if (fd_write < 0) {
        current->free_fd(fd_read);
        goto failed;
    }

    fd[0] = fd_read;
    fd[1] = fd_write;
    return 0;

failed:
    close_file(file_read);
    close_file(file_write);
    return -1;
}

int file_system_t::do_send_to(int fd, void* buffer, uint32 count, sock_addr_t* addr)
{
    file_t* file = current->get_file(fd);
    if (file == NULL || file->m_readable == 0) {
        return -1;
    }

    if (file->m_type == file_t::TYPE_SOCKET) {
        return file->m_socket->send_to(buffer, count, addr);
    }

    return -1;
}

int file_system_t::do_recv_from(int fd, void* buffer, uint32 count, sock_addr_t* addr)
{
    file_t* file = current->get_file(fd);
    if (file == NULL || file->m_readable == 0) {
        return -1;
    }

    if (file->m_type == file_t::TYPE_SOCKET) {
        return file->m_socket->recv_from(buffer, count, addr);
    }

    return -1;
}

