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

// only for test
void test_fork_exec_wait_exit(const char* times)
{
    int t = 0;
    while (*times != '\0') {
        if (*times < '0' || *times > '9') {
            break;
        }
        t = t*10 + *times - '0';
        times++;
    }

    argument.m_argc = 1;
    argument.m_argv[0][0] = 0;
    for (int i = 0; i < t; i++) {
        int32 pid = userlib_t::fork();
        if (pid == 0) {
            int ret = userlib_t::exec("/bin/ls", &argument);
            if (ret < 0) {
                userlib_t::exit(-1);
            }
            userlib_t::exit(0);
        }

        userlib_t::wait(pid);
    }
}

char cmd_line[MAX_CMD_LEN] = {0};
int main()
{
    userlib_t::printf("This is printed by shell.\n");

    while (true) {
        puts("liuruyi $ ");
        gets(cmd_line, MAX_CMD_LEN);
        if (userlib_t::strncmp(cmd_line, "cd ", 3) == 0) {
            if (userlib_t::chdir(cmd_line + 3) < 0) {
                userlib_t::printf("can't cd %s\n", cmd_line+3);
            }
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "test ", 5) == 0) {
            test_fork_exec_wait_exit(cmd_line + 5);
            continue;
        }

        do_cmd(cmd_line);
    }

    userlib_t::exit(0);
    return 1;
}

