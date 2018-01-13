/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"
#include "file.h"
#include "types.h"

#define MAX_CMD_LEN 128

void gets(char* buf, uint32 max)
{
    userlib_t::memset(buf, 0, max);
    int i = 0;
    while (i < max) {
        char c;
        int n = userlib_t::read(0, &c, 1);
        if (n == 1) {
            if (c == '\n') {
                break;
            }
            *buf++ = c;
            i++;
        }
    }
}

void puts(char* buf)
{
    userlib_t::write(0, buf, userlib_t::strlen(buf));
}

static char command[MAX_CMD_LEN] = {0};
static argument_t argument;

const char* parse_cmd(const char* cmd_line, char* cmd)
{
    userlib_t::memset(cmd, 0, MAX_CMD_LEN);
    if (*cmd_line != '/') {
        userlib_t::strcpy(cmd, "/bin/");
        cmd += userlib_t::strlen(cmd);
    }

    const char* p = cmd_line;
    while (*p != ' ' && *p != '\0') {
        *cmd++ = *p++;
    }

    while (*p == ' ') {
        p++;
    }

    return p;
}

void parse_cmd_line(const char* cmd_line, char* cmd, argument_t* arg)
{
    const char* p = parse_cmd(cmd_line, cmd);
    arg->m_argc = 0;
    char* q = arg->m_argv[arg->m_argc++];
    userlib_t::strcpy(q, cmd);

    if (*p == '\0') {
        return;
    }

    while (true) {
        char* q = arg->m_argv[arg->m_argc];
        userlib_t::memset(q, 0, MAX_ARG_LEN);
        while (*p != ' ' && *p != '\0') {
            *q++ = *p++;
        }

        while (*p == ' ') {
            p++;
        }

        arg->m_argc++;
        if (*p == '\0') {
            break;
        }
    }
}

void do_cmd(const char* cmd_line)
{
    parse_cmd_line(cmd_line, command, &argument);
    int32 pid = userlib_t::fork();
    if (pid == 0) {
        int ret = userlib_t::exec(command, &argument);
        if (ret < 0) {
            userlib_t::exit(-1);
        }
    }

    userlib_t::wait(pid);
}

char cmd_line[MAX_CMD_LEN] = {0};
int main()
{
    userlib_t::printf("This is printed by shell.\n");

    while (true) {
        puts("liuruyi $ ");
        gets(cmd_line, MAX_CMD_LEN);
        if (cmd_line[0] == 'c' && cmd_line[1] == 'd' && cmd_line[2] == ' ') {
            if (userlib_t::chdir(cmd_line+3) < 0) {
                userlib_t::printf("can't cd %s\n", cmd_line+3);
            }
            continue;
        }

        do_cmd(cmd_line);
    }

    userlib_t::exit(0);
    return 1;
}

#if 0
int main()
{
    int fd = userlib_t::open("/test", file_t::MODE_RDWR);
    if (fd < 0) {
        userlib_t::print("failed to open file abc\n");
        userlib_t::exit(-1);
    }

    char buf[512] = {0};
    int n = userlib_t::read(fd, buf, 512);
    userlib_t::print_int(n, 10, 1);
    userlib_t::print(" bytes read from file.\n");
    userlib_t::print(buf);

    while (true) {
        int n = userlib_t::read(fd, buf, 512);
        if (n > 0) {
            userlib_t::print_int(n, 10, 1);
            userlib_t::print(" bytes read from file.\n");
        }
    }

    userlib_t::close(fd);
    userlib_t::exit(0);
    return 1;
}

void process_signal(int32 sig)
{
    userlib_t::print("SIG_");
    userlib_t::print_int(sig, 10, 1);
    userlib_t::print(",");
}

int main()
{
    userlib_t::exit(0);
    return 1;

    sighandler_t handler = &process_signal;
    userlib_t::signal(4, handler);
    userlib_t::print("This is printed by shell.\n");

    int times = 0;
    while (times++ < 5) {
        userlib_t::sleep(2);
        userlib_t::print("S,");
    }

    userlib_t::exit(0);

    int* p = (int *) 0xa0000000;
    *p = 0x1234;

    while (1) {
        userlib_t::sleep(2);
        userlib_t::print("s,");
    }

    userlib_t::exit(0);
    return 1;
}
#endif

