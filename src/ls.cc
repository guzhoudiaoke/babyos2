/*
 * guzhoudiaoke@126.com
 * 2018-01-12
 */

#include "userlib.h"

void get_name(const char* path, char* name)
{
    const char* p = path + userlib_t::strlen(path);
    while (*p == '/') {
        p--;
    }
    while (p >= path && *p != '/') {
        p--;
    }
    p++;

    userlib_t::strcpy(name, p);
}

void list_file(const char* name, uint32 size)
{
    userlib_t::printf("%20s %u\n", name, size);
}

void ls(const char* path)
{
    int fd = userlib_t::open(path, file_t::MODE_RDONLY);
    if (fd < 0) {
        userlib_t::printf("ls: cannot open %s\n", path);
        return;
    }

    stat_t st;
    if (userlib_t::fstat(fd, &st) < 0) {
        userlib_t::printf("ls: cannot stat file %s\n", fd);
        return;
    }

    static char name[32] = {0};
    dir_entry_t de;

    switch (st.m_type) {
    case inode_t::I_TYPE_FILE:
        list_file(name, st.m_size);
        break;
    case inode_t::I_TYPE_DIR:
        userlib_t::printf("%s: \n", path);
        while (userlib_t::read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.m_inum == 0) {
                continue;
            }

            char p[128] = {0};
            if (userlib_t::strcmp(de.m_name, ".") != 0 && userlib_t::strcmp(de.m_name, "..") != 0) {
                userlib_t::strcpy(p, path);
                if (*(p + userlib_t::strlen(p)) != '/') {
                    userlib_t::strcat(p, "/");
                }
            }
            userlib_t::strcat(p, de.m_name);

            if (userlib_t::stat(p, &st) < 0) {
                userlib_t::printf("ls: cannot stat %s\n", p);
                continue;
            }

            list_file(de.m_name, st.m_size);
        }
        break;
    }

    userlib_t::close(fd);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        ls(".");
        userlib_t::exit(0);
    }

    for (int i = 1; i < argc; i++) {
        ls(argv[i]);
    }

    userlib_t::exit(0);
    return 0;
}

