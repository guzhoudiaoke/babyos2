/*
 * split from class fs
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#include "fs_test.h"
#include "babyos.h"
#include "string.h"

void fs_tester_t::init(file_system_t* fs)
{
    m_fs = fs;
}

void fs_tester_t::dump_super_block()
{
    console()->kprintf(YELLOW, "super block: \n");
    console()->kprintf(YELLOW, "size: %u block num: %u inode num: %u \n", 
            m_fs->m_super_block.m_size, m_fs->m_super_block.m_nblocks, m_fs->m_super_block.m_ninodes);
}

void fs_tester_t::test_read_inode()
{
    inode_t inode;
    inode.m_valid = 0;
    inode.m_sem.init(1);
    console()->kprintf(WHITE, "inodes: \n");
    for (int i = 0; i < m_fs->m_super_block.m_ninodes; i++) {
        inode.read_from_disk(i);
        if (inode.m_type != 0) {
            console()->kprintf(WHITE, "%d: { %u, %u, %u, %u, %u }\n", 
                    i, inode.m_type, inode.m_major,
                    inode.m_minor, inode.m_nlinks, inode.m_size);
        }
    }
}

void fs_tester_t::test_read_bitmap()
{
    uint32 index = m_fs->bitmap_block();
    console()->kprintf(YELLOW, "bitmap: \n");
    for (unsigned int i = 0; i < m_fs->m_super_block.m_nblocks; i += BSIZE*8) {
        io_buffer_t* b = os()->get_block_dev()->read(index);
        uint32 size = BSIZE * 8;
        if (size > m_fs->m_super_block.m_nblocks - i) {
            size = m_fs->m_super_block.m_nblocks - i;
        }

        uint32* bmp = (uint32 *) b->m_buffer;
        for (int j = 0; j < size / 32; j++) {
            console()->kprintf(YELLOW, "%x, ", bmp[j]);
            if (j % 8 == 0) {
                console()->kprintf(YELLOW, "\n");
            }

        }
        os()->get_block_dev()->release_block(b);
        index++;
    }
    console()->kprintf(YELLOW, "\n");
}

void fs_tester_t::test_read_dir_entry()
{
    console()->kprintf(WHITE, "read dir entry: \n");
    inode_t inode;
    inode.m_valid = 0;
    inode.m_sem.init(1);
    for (int i = 0; i < m_fs->m_super_block.m_ninodes; i++) {
        inode.read_from_disk(i);
        if (inode.m_type == inode_t::I_TYPE_DIR) {
            break;
        }
    }

    dir_entry_t dir;
    unsigned int offset = 0;
    while (m_fs->read_inode(&inode, (char *) &dir, offset, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
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

void fs_tester_t::test_namei()
{
    console()->kprintf(YELLOW, "test namei: \n");
    inode_t* inode = m_fs->namei("/bin/ls");
    if (inode != NULL) {
        console()->kprintf(YELLOW, "{ %u, %u, %u, %u, %u }\n", inode->m_type, inode->m_major,
                inode->m_minor, inode->m_nlinks, inode->m_size);
        m_fs->put_inode(inode);
    }
    else {
        console()->kprintf(YELLOW, "NULL\n");
    }
}

void fs_tester_t::test_create()
{
    console()->kprintf(YELLOW, "before create file \"test\" \n");
    test_read_dir_entry();

    int fd = m_fs->do_open("/test", file_t::MODE_CREATE);
    m_fs->do_close(fd);

    console()->kprintf(YELLOW, "after create file \"test\" \n");
    test_read_dir_entry();
}

void fs_tester_t::test_read()
{
    console()->kprintf(YELLOW, "test read: \n");
    int fd = m_fs->do_open((char *) "/test", 0);
    if (fd < 0) {
        return;
    }

    char buffer[512] = {0};
    int n = m_fs->do_read(fd, buffer, 512);
    console()->kprintf(YELLOW, "%u bytes read from file test: \n%s\n", n, buffer);
    m_fs->do_close(fd);
}

void fs_tester_t::test_write()
{
    console()->kprintf(PINK, "test write: \n");
    int fd = m_fs->do_open("/test", file_t::MODE_RDWR);
    if (fd < 0) {
        return;
    }

    // read
    char buffer[512] = {0};
    int n = m_fs->do_read(fd, buffer, 512);
    console()->kprintf(WHITE, "%u bytes read from file test: \n%s\n", n, buffer);
    m_fs->do_close(fd);

    // write
    strcpy(buffer + 37, "2) test write...\n");
    fd = m_fs->do_open((char *) "/test", file_t::MODE_RDWR);
    n = m_fs->do_write(fd, buffer, strlen(buffer));
    console()->kprintf(WHITE, "%u bytes write to file\n", n);
    m_fs->do_close(fd);

    // re-read
    fd = m_fs->do_open((char *) "/test", file_t::MODE_RDWR);
    memset(buffer, 0, 512);
    n = m_fs->do_read(fd, buffer, 512);
    console()->kprintf(WHITE, "%u bytes read from file test: \n%s\n", n, buffer);
    m_fs->do_close(fd);
}

void fs_tester_t::test_mkdir()
{
    console()->kprintf(PINK, "test mkdir: \n");
    m_fs->do_mkdir("/test_mkdir");

    int fd = m_fs->do_open("/test_mkdir/abc", file_t::MODE_CREATE | file_t::MODE_RDWR);
    if (fd < 0) {
        console()->kprintf(PINK, "failed to create abc in dir test_mkdir\n");
        return;
    }

    char buffer[64] = {0};
    strcpy(buffer, "test create file in new maked dir\n");
    int n = m_fs->do_write(fd, buffer, strlen(buffer));
    console()->kprintf(YELLOW, "%u bytes write to file\n", n);
    m_fs->do_close(fd);

    fd = m_fs->do_open((char *) "/test_mkdir/abc", file_t::MODE_RDWR);
    if (fd < 0) {
        console()->kprintf(PINK, "failed to open new file\n");
        return;
    }
    do {
        memset(buffer, 0, 64);
        n = m_fs->do_read(fd, buffer, 64);
        if (n > 0) {
            console()->kprintf(YELLOW, "%u bytes read from file test: \n%s\n", n, buffer);
        }
    } while (n > 0);
    m_fs->do_close(fd);

    test_ls("/");
    test_ls("/test_mkdir/");

    return;

}

void fs_tester_t::test_ls(const char* path)
{
    console()->kprintf(GREEN, "\n%s:\n", path);
    inode_t* inode = m_fs->namei(path);
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
        while (m_fs->read_inode(inode, (char *) &dir, offset, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
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

    m_fs->put_inode(inode);
}

void fs_tester_t::test_link()
{
    console()->kprintf(PINK, "test link /test to test_mkdir/test.ln: \n");
    m_fs->do_link("/test", "/test_mkdir/test.ln");
    test_ls("/");
    test_ls("/test_mkdir/");
}

void fs_tester_t::test_unlink()
{
    console()->kprintf(PINK, "test unlink /test_mkdir/abc: \n");
    m_fs->do_unlink("/test_mkdir/abc");
    test_ls((char *) "/test_mkdir/");

    console()->kprintf(PINK, "test unlink /test_mkdir: \n");
    if (m_fs->do_unlink("/test_mkdir") != 0) {
        console()->kprintf(PINK, "failed to unlink /test_mkdir: \n");
    }
    else {
        console()->kprintf(WHITE, "success to unlink /test_mkdir: \n");
    }

    console()->kprintf(PINK, "test unlink /test_mkdir/test.ln: \n");
    m_fs->do_unlink("/test_mkdir/test.ln");
    test_ls((char *) "/test_mkdir/");

    if (m_fs->do_unlink("/test_mkdir") != 0) {
        console()->kprintf(PINK, "failed to unlink /test_mkdir: \n");
    }
    else {
        console()->kprintf(WHITE, "success to unlink /test_mkdir: \n");
    }
}

