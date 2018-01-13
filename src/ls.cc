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
    userlib_t::print(name);
    for (int i = 0; i < 20 - userlib_t::strlen(name); i++) {
        userlib_t::print(" ");
    }
    userlib_t::print_int(size, 10, 0);
    userlib_t::print("\n");
}

void ls(const char* path)
{
    int fd = userlib_t::open(path, file_t::MODE_RDONLY);
    if (fd < 0) {
        userlib_t::print("ls: cannot open ");
        userlib_t::print(path);
        userlib_t::print("\n");
        return;
    }

    stat_t st;
    if (userlib_t::fstat(fd, &st) < 0) {
        userlib_t::print("ls: cannot stat ");
        userlib_t::print(path);
        userlib_t::close(fd);
        return;
    }

    static char name[32] = {0};
    dir_entry_t de;

    switch (st.m_type) {
    case inode_t::I_TYPE_FILE:
        list_file(name, st.m_size);
        break;
    case inode_t::I_TYPE_DIR:
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
                userlib_t::print("ls: cannot stat ");
                userlib_t::print(p);
                userlib_t::print("\n");
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
    ls("/");
    userlib_t::exit(0);
    return 0;

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
