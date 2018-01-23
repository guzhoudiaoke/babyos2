/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"
#include "file.h"
#include "types.h"
#include "socket_local.h"

#define MAX_CMD_LEN 128

argument_t argument;

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

static char command[MAX_CMD_LEN] = {0};
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
static void test_fork_exec_wait_exit(const char* times)
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

static void test_pipe()
{
    int fd[2];
    if (userlib_t::pipe(fd) < 0) {
        userlib_t::printf("failed to create pipe\n");
        return;
    }

    userlib_t::printf("succ to create pipe: %d, %d\n", fd[0], fd[1]);

    int32 pid = userlib_t::fork();
    if (pid == 0) {
        userlib_t::close(fd[0]);

        char ch = 'a';
        for (int i = 0; i < 10; i++, ch++) {
            userlib_t::write(fd[1], &ch, 1);
            userlib_t::printf("child write %c to pipe\n", ch);
            userlib_t::sleep(1);
        }
        userlib_t::exit(0);
    }
    else {
        userlib_t::close(fd[1]);

        char ch = '\0';
        for (int i = 0; i < 10; i++) {
            userlib_t::read(fd[0], &ch, 1);
            userlib_t::printf("parent read %c from pipe\n", ch);
        }
        userlib_t::wait(pid);
    }
}

static void do_server(int sockfd)
{
    int data = 0;
    if (userlib_t::read(sockfd, &data, sizeof(int)) < 0) {
        userlib_t::printf("server read error.\n");
        return;
    }
    userlib_t::printf("server read  %d from client.\n", data);
    data++;

    if (userlib_t::write(sockfd, &data, sizeof(int)) < 0) {
        userlib_t::printf("server write error.\n");
        return;
    }
    userlib_t::printf("server write %d to   client.\n", data);
}

static void socket_server()
{
    int listen_fd = userlib_t::socket(socket_t::AF_LOCAL, 0, 0);
    if (listen_fd < 0) {
        userlib_t::printf("err, server create socket failed, error %u\n", listen_fd);
        return;
    }
    userlib_t::printf("server create socket succ: %u\n", listen_fd);

    sock_addr_local_t addr;
    addr.m_family = socket_t::AF_LOCAL;
    userlib_t::strcpy(addr.m_path, "/test_socket");

    int ret = 0;
    if ((ret = userlib_t::bind(listen_fd, &addr)) < 0) {
        userlib_t::printf("err, server bind to %u failed, error %u\n", listen_fd, ret);
        return;
    }
    userlib_t::printf("server bind succ\n");

    if ((ret = userlib_t::listen(listen_fd, 1)) < 0) {
        userlib_t::printf("err, server listen failed, error %u\n", ret);
        return;
    }
    userlib_t::printf("server listen succ\n");

    int conn_fd = -1;
    sock_addr_local_t client_addr;
    for (int i = 0; i < 2; i++) {
        if ((conn_fd = userlib_t::accept(listen_fd, &client_addr)) < 0) {
            userlib_t::printf("accept failed.\n");
            continue;
        }

        userlib_t::printf("server accept success: %u\n", conn_fd);
        if (userlib_t::fork() == 0) {
            userlib_t::close(listen_fd);
            do_server(conn_fd);
            userlib_t::sleep(1);
            userlib_t::exit(0);
        }
        else {
            userlib_t::close(conn_fd);
        }
    }
}

static void do_client(int sockfd, int data)
{
    userlib_t::write(sockfd, &data, sizeof(int));
    userlib_t::printf("client write %d to   server.\n", data);

    userlib_t::read(sockfd, &data, sizeof(int));
    userlib_t::printf("client read  %d from server.\n", data);
}

static void socket_client(int data)
{
    int sock_fd = userlib_t::socket(socket_t::AF_LOCAL, 0, 0);
    if (sock_fd < 0) {
        userlib_t::printf("client create socket failed, error %u\n", sock_fd);
        return;
    }
    userlib_t::printf("client create socket success, fd: %u\n", sock_fd);


    sock_addr_local_t addr;
    addr.m_family = socket_t::AF_LOCAL;
    userlib_t::strcpy(addr.m_path, "/test_socket");

    int ret = 0;
    if ((ret = userlib_t::connect(sock_fd, &addr)) < 0) {
        userlib_t::printf("client connect to fd: %u failed, error %u\n", sock_fd, ret);
        return;
    }

    userlib_t::printf("client connect success\n");
    do_client(sock_fd, data);
}

static void test_socket()
{
    int32 pid1 = -1; 
    int32 pid2 = -1;
    int32 pid3 = -1;

    pid1 = userlib_t::fork();
    if (pid1 == 0) {
        /* server */
        socket_server();
        userlib_t::exit(0);
    }

    userlib_t::sleep(1);
    pid2 = userlib_t::fork();
    if (pid2 == 0) {
        /* client */
        socket_client(1234);
        userlib_t::sleep(1);
        userlib_t::exit(0);
    }

    userlib_t::sleep(1);
    pid3 = userlib_t::fork();
    if (pid3 == 0) {
        /* client */
        socket_client(5678);
        userlib_t::sleep(1);
        userlib_t::exit(0);
    }

    /* shell */
    userlib_t::wait(pid1);
    userlib_t::wait(pid2);
    userlib_t::wait(pid3);
}


int main()
{
    char cmd_line[MAX_CMD_LEN] = {0};
    userlib_t::printf("This is printed by shell.\n");

    while (true) {
        //userlib_t::puts("liuruyi $ ");
        userlib_t::color_print(GREEN, "liuruyi $ ");
        userlib_t::gets(cmd_line, MAX_CMD_LEN);
        if (userlib_t::strncmp(cmd_line, "cd ", 3) == 0) {
            if (userlib_t::chdir(cmd_line + 3) < 0) {
                userlib_t::printf("can't cd %s\n", cmd_line+3);
            }
            continue;
        }

        /* used for test */
        if (userlib_t::strncmp(cmd_line, "test ", 5) == 0) {
            test_fork_exec_wait_exit(cmd_line + 5);
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "testpipe", 8) == 0) {
            test_pipe();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "testsocket", 10) == 0) {
            test_socket();
            continue;
        }

        do_cmd(cmd_line);
    }

    userlib_t::exit(0);
    return 1;
}

