/*
 * guzhoudiaoke@126.com
 * 2018-01-02
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "file.h"

file_system_t fs;
process_t* current;

int sys_open(char* path, uint32 mode)
{
    return fs.do_open(path, mode);
}

int sys_close(int fd)
{
    return fs.do_close(fd);
}

int sys_read(int fd, void* buffer, unsigned count)
{
    return fs.do_read(fd, buffer, count);
}

int sys_write(int fd, void* buffer, unsigned count)
{
    return fs.do_write(fd, buffer, count);
}

int sys_mkdir(char* path)
{
    return fs.do_mkdir(path);
}

int sys_link(char* path_old, char* path_new)
{
    return fs.do_link(path_old, path_new);
}

int sys_unlink(char* path)
{
    return fs.do_unlink(path);
}

void test_superblock()
{
    printf("size: %u, nblocks: %u, ninode: %u\n", fs.m_super_block.m_size,
            fs.m_super_block.m_nblocks,
            fs.m_super_block.m_ninodes);
}

void test_bitmap()
{
    fs.m_block_bmp.dump();
}

void test_inode()
{
    inode_t inode;
    for (int i = 0; i < fs.m_super_block.m_ninodes; i++) {
        fs.read_disk_inode(i, &inode);
        if (inode.m_type != 0) {
            printf("%d: { %u, %u, %u, %u, %u }\n", i, inode.m_type, inode.m_major,
                    inode.m_minor, inode.m_nlinks, inode.m_size);
        }
    }
}

void test_read_inode()
{
    inode_t inode;
    for (int i = 0; i < fs.m_super_block.m_ninodes; i++) {
        fs.read_disk_inode(i, &inode);
        if (inode.m_type == 1) {
            break;
        }
    }

    dir_entry_t dir;
    unsigned int offset = 0;
    while (true) {
        if (fs.read_inode(&inode, (char *) &dir, offset, sizeof(dir_entry_t)) != sizeof(dir_entry_t)) {
            break;
        }

        if (dir.m_inum != 0) {
            printf("%s\t inum: %u\n", dir.m_name, dir.m_inum);
        }

        offset += sizeof(dir_entry_t);
    }
}

void test_namei()
{
    char cmd[MAX_PATH] = "ls";
    inode_t* inode = fs.namei(cmd);
    if (inode != NULL) {
        printf("{ %u, %u, %u, %u, %u }\n", inode->m_type, inode->m_major,
                inode->m_minor, inode->m_nlinks, inode->m_size);
    }
    else {
        printf("NULL\n");
    }
    fs.put_inode(inode);
}

void test_read()
{
    int fd = sys_open((char *) "/test", 0);
    if (fd < 0) {
        printf("failed to open\n");
        return;
    }

    char buffer[512] = {0};
    int n = sys_read(fd, buffer, 512);
    printf("%u bytes read from file test: \n%s\n", n, buffer);
    sys_close(fd);
}

void test_read2()
{
    int fd = sys_open((char *) "/zombie.asm", 0);
    if (fd < 0) {
        printf("failed to open\n");
        return;
    }

    FILE* fp = fopen((char *) "z.asm", "w");

    char buffer[512] = {0};
    int n = sys_read(fd, buffer, 512);
    while (n > 0) {
        fwrite(buffer, 1, n, fp);
        n = sys_read(fd, buffer, 512);
    }

    fclose(fp);
    sys_close(fd);
}

void test_create()
{
    int fd = sys_open((char *) "/abc", file_t::MODE_CREATE);
    sys_close(fd);
}

void test_write1()
{
    int fd = sys_open((char *) "/test", file_t::MODE_RDWR);
    if (fd < 0) {
        printf("failed to open\n");
        return;
    }

    char buffer[512] = {0};
    int n = sys_read(fd, buffer, 512);
    while (n > 0) {
        printf("%u bytes read from file test: \n%s\n", n, buffer);
        n = sys_read(fd, buffer, 512);
    }
    sys_close(fd);

    strcpy(buffer + 37, "2) test write...\n");
    fd = sys_open((char *) "/test", file_t::MODE_RDWR);
    n = sys_write(fd, buffer, strlen(buffer));
    printf("%u bytes write to file\n", n);
    sys_close(fd);

    fd = sys_open((char *) "/test", file_t::MODE_RDWR);
    memset(buffer, 0, 512);
    n = sys_read(fd, buffer, 512);
    while (n > 0) {
        printf("%u bytes read from file test: \n%s\n", n, buffer);
        n = sys_read(fd, buffer, 512);
    }
    sys_close(fd);
}

void test_write2()
{
    int fd = sys_open((char *) "/write2", file_t::MODE_CREATE | file_t::MODE_RDWR);
    if (fd < 0) {
        printf("failed to open\n");
        return;
    }

    char buffer[4096] = {0};
    char c = 0;
    for (int i = 0; i < 4096; i++) {
        buffer[i] = c + 'a';
        c = (c + 1) % 26;
    }

    int n = sys_write(fd, buffer, 4096);
    printf("%u bytes write to file\n", n);
    sys_close(fd);

    fd = sys_open((char *) "/write2", file_t::MODE_RDWR);
    do {
        memset(buffer, 0, 4096);
        n = sys_read(fd, buffer, 512);
        if (n > 0) {
            printf("%u bytes read from file test: \n%s\n", n, buffer);
        }
    } while (n > 0);
    sys_close(fd);
}

void test_mkdir()
{
    sys_mkdir((char *) "/test_mkdir");
    int fd = sys_open((char *) "/test_mkdir/abc", file_t::MODE_CREATE | file_t::MODE_RDWR);
    if (fd < 0) {
        printf("failed to open\n");
        return;
    }

    char buffer[64] = {0};
    strcpy(buffer, "test create file in new maked dir\n");
    int n = sys_write(fd, buffer, strlen(buffer));
    printf("%u bytes write to file\n", n);
    sys_close(fd);

    fd = sys_open((char *) "/test_mkdir/abc", file_t::MODE_RDWR);
    do {
        memset(buffer, 0, 64);
        n = sys_read(fd, buffer, 64);
        if (n > 0) {
            printf("%u bytes read from file test: \n%s\n", n, buffer);
        }
    } while (n > 0);
    sys_close(fd);
}

void ls(char* path)
{
    printf("\n%s:\n", path);
    inode_t* inode = fs.namei(path);
    if (inode == NULL) {
        printf("failed to ls %s\n", path);
        return;
    }

    if (inode->m_type == inode_t::I_TYPE_FILE) {
        printf("%s %d %d %d %d\n", path, inode->m_type, inode->m_inum, inode->m_size);
    }
    else if (inode->m_type == inode_t::I_TYPE_DIR) {
        dir_entry_t dir;
        unsigned int offset = 0;
        while (fs.read_inode(inode, (char *) &dir, offset, sizeof(dir_entry_t)) == sizeof(dir_entry_t)) {
            if (dir.m_inum != 0) {
                printf("%s\t %u\n", dir.m_name, dir.m_inum);
            }
            offset += sizeof(dir_entry_t);
            memset(dir.m_name, 0, MAX_PATH);
        }
    }

    fs.put_inode(inode);
}

void test_link()
{
    sys_link((char *) "/test", (char *) "/test_mkdir/test.ln");
    ls((char *) "/");
    ls((char *) "/test_mkdir/");
}

void test_unlink(char* path)
{
    sys_unlink(path);
    ls((char *) "/");
}

void get_name(char* path, char* name)
{
    char* p = path + strlen(path);
    char* end;
    while (*p == '/') {
        p--;
    }
    end = p;
    while (*p != '/') {
        p--;
    }

    strncpy(name, p+1, end-p);
}

void check(char* path)
{
    int fd = sys_open(path, file_t::MODE_RDWR);
    if (fd < 0) {
        printf("failed to open %s\n", path);
        return;
    }

    char name[128] = {0};
    get_name(path, name);
    FILE* fp = fopen(name, "w");
    if (fp == NULL) {
        printf("failed to open %s\n", name);
        return;
    }

    char buffer[512];
    int nbyte;
    do {
        nbyte = sys_read(fd, buffer, 512);
        fwrite(buffer, 1, nbyte, fp);
    } while (nbyte > 0);

    fclose(fp);
    sys_close(fd);
}

void write_file(char* path)
{
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        return;
    }

    char name[128] = {0};
    get_name(path, name);

    char disk_path[128] = {0};
    sprintf(disk_path, "/bin/%s", name);

    test_unlink(disk_path);

    int fd = sys_open(disk_path, file_t::MODE_CREATE | file_t::MODE_RDWR);

    char buffer[512];
    int nbyte;
    int total_w = 0, total_r = 0;
    do {
        nbyte = fread(buffer, 1, 512, fp);
        total_r += nbyte;
        int nwrite = sys_write(fd, buffer, nbyte);
        total_w += nwrite;

        //printf("%d %d \n", nbyte, nwrite);
    } while (nbyte > 0);

    printf("%s, total read: %u, total write %u\n", disk_path, total_r, total_w);
    sys_close(fd);
    fclose(fp);

    ls("/bin");

    check(disk_path);
}

int main()
{
    process_t proc;
    proc.init();
    current = &proc;

    fs.init();


    test_superblock();

    //test_unlink("/rm");
    //test_unlink("/sh");
    //test_unlink("/cat");
    //test_unlink("/mkdir");
    //test_unlink("/ls");
    //test_unlink("/wc");
    //test_unlink("/echo");
    //test_unlink("/init");
    //test_unlink("/kill");
    //test_unlink("/grep");
    //test_unlink("/test");
    //test_unlink("/abc");
    //test_unlink("/ln");
    //test_unlink("/zombie.asm");

    //test_bitmap();
    //test_inode();
    //test_read_inode();
    //test_namei();

    //test_read();
    //test_read2();

    //test_create();
    //test_read_inode();

    //test_write1();
    //test_write2();

    //test_mkdir();

    //test_link();
    //test_unlink();

    sys_mkdir("/bin");
    write_file("../../src/init");
    write_file("../../src/shell");

    //ls("/");
    return 0;
}

