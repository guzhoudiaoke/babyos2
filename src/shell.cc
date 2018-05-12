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

// only for test
static void test_fork_wait_exit(const char* times)
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
            //if (i % 100 == 0) {
                userlib_t::printf("%u\n", i);
            //}
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

/********************* socket local *******************/

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

/********************* socket local *******************/

/************************ udp ************************/
const int TEST_UDP_PORT = 12345;
static void udp_server()
{
    int sock_fd = userlib_t::socket(socket_t::AF_INET, socket_t::SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        userlib_t::printf("ERROR, server create socket failed, error %u\n", sock_fd);
        return;
    }
    userlib_t::printf("server create socket success, fd: %u\n", sock_fd);

    sock_addr_inet_t addr;
    addr.m_family = socket_t::AF_INET;
    addr.m_ip = userlib_t::htonl(sock_addr_inet_t::INADDR_ANY);
    addr.m_port = userlib_t::htons(TEST_UDP_PORT);
    if (userlib_t::bind(sock_fd, &addr) < 0) {
        userlib_t::printf("ERROR, server bind failed\n");
        return;
    }
    userlib_t::printf("server bind success\n");

    char buffer[512] = {0};
    for (; ; ) {
        userlib_t::memset(buffer, 0, 512);
        sock_addr_inet_t addr_client;
        int ret = userlib_t::recv_from(sock_fd, buffer, 512, &addr_client);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to recv_from, error %u\n", ret);
            break;
        }

        uint32 ip = userlib_t::ntohl(addr_client.m_ip);
        uint8* p = (uint8 *) &ip;
        userlib_t::printf("server receive from %u.%u.%u.%u, %u: %s\n", 
                p[3], p[2], p[1], p[0], userlib_t::ntohs(addr_client.m_port), buffer);

        ret = userlib_t::send_to(sock_fd, buffer, userlib_t::strlen(buffer), &addr_client);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to send_to, error %u\n", ret);
            break;
        }
    }
}

static void test_udp_server()
{
    int32 pid = userlib_t::fork();
    if (pid == 0) {
        /* server */
        udp_server();
        userlib_t::exit(0);
    }

    /* shell */
    userlib_t::wait(pid);
}

static void udp_client()
{
    int sock_fd = userlib_t::socket(socket_t::AF_INET, socket_t::SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        userlib_t::printf("ERROR, client create socket failed, error %u\n", sock_fd);
        return;
    }
    userlib_t::printf("client create socket success, fd: %u\n", sock_fd);

    sock_addr_inet_t addr;
    addr.m_family = socket_t::AF_INET;
    addr.m_ip = userlib_t::htonl(userlib_t::make_ipaddr(192, 168, 1, 105));
    addr.m_port = userlib_t::htons(TEST_UDP_PORT);

    char buffer[512] = {0};
    for (int i = 0; i < 5; i++) {
        userlib_t::memset(buffer, 0, 512);
        userlib_t::gets(buffer, 512);
        int ret = userlib_t::send_to(sock_fd, buffer, userlib_t::strlen(buffer), &addr);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to send_to, error %u\n", ret);
            break;
        }

        userlib_t::memset(buffer, 0, 512);
        sock_addr_inet_t addr_recv;
        ret = userlib_t::recv_from(sock_fd, buffer, 512, &addr_recv);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to recv_from, error %u\n", ret);
            break;
        }

        userlib_t::printf("receive: %s\n", buffer);
    }
}

static void test_udp_client()
{
    int32 pid = userlib_t::fork();
    if (pid == 0) {
        /* server */
        udp_client();
        userlib_t::exit(0);
    }

    /* shell */
    userlib_t::wait(pid);
}

/************************ udp ************************/


/************************ nslookup ************************/

void ns_lookup(const char* name)
{
    uint32 ip = userlib_t::get_ip_by_name(name);
    uint8* p = (uint8 *) &ip;
    userlib_t::printf("IP: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
}

/************************ nslookup ************************/

/************************ tcp ************************/
static void tcp_server_echo(int conn_fd)
{
    char buffer[512] = {0};
    while (1) {
        userlib_t::memset(buffer, 0, 512);
        int ret = userlib_t::read(conn_fd, buffer, 512);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to read, error %u\n", ret);
            break;
        }
        userlib_t::printf("server read %d bytes from client: %s\n", ret, buffer);

        ret = userlib_t::write(conn_fd, buffer, userlib_t::strlen(buffer));
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to write, error %u\n", ret);
            break;
        }
        userlib_t::printf("server write %d bytes to client.\n", ret);
    }
    userlib_t::printf("break from while\n");
}

const int TEST_TCP_PORT = 56789;
static void tcp_server()
{
    int sock_fd = userlib_t::socket(socket_t::AF_INET, socket_t::SOCK_STREAM, 0);
    if (sock_fd < 0) {
        userlib_t::printf("ERROR, server create socket failed, error %u\n", sock_fd);
        return;
    }
    userlib_t::printf("server create socket success, fd: %u\n", sock_fd);

    sock_addr_inet_t addr;
    addr.m_family = socket_t::AF_INET;
    addr.m_ip = userlib_t::htonl(sock_addr_inet_t::INADDR_ANY);
    addr.m_port = userlib_t::htons(TEST_TCP_PORT);

    int ret = 0;
    if ((ret = userlib_t::bind(sock_fd, &addr)) < 0) {
        userlib_t::printf("ERROR, server bind failed: %u\n", ret);
        return;
    }
    userlib_t::printf("server bind success\n");

    if ((ret = userlib_t::listen(sock_fd, 5)) < 0) {
        userlib_t::printf("err, server listen failed, error %u\n", ret);
        return;
    }
    userlib_t::printf("server listen succ\n");

    int conn_fd = -1;
    sock_addr_local_t client_addr;
    for (int i = 0; i < 5; i++) {
        if ((conn_fd = userlib_t::accept(sock_fd, &client_addr)) < 0) {
            userlib_t::printf("accept failed %u\n", -conn_fd);
            continue;
        }
        userlib_t::printf("server accept success: %u\n", conn_fd);
        if (userlib_t::fork() == 0) {
            userlib_t::close(sock_fd);
            tcp_server_echo(conn_fd);
            //userlib_t::close(conn_fd);
            userlib_t::exit(0);
        }
        else {
            userlib_t::close(conn_fd);
        }
    }
}

static void tcp_client()
{
    int sock_fd = userlib_t::socket(socket_t::AF_INET, socket_t::SOCK_STREAM, 0);
    if (sock_fd < 0) {
        userlib_t::printf("ERROR, client create socket failed, error %u\n", -sock_fd);
        return;
    }
    userlib_t::printf("client create socket success, fd: %u\n", sock_fd);

    sock_addr_inet_t addr;
    addr.m_family = socket_t::AF_INET;
    addr.m_ip = userlib_t::htonl(userlib_t::make_ipaddr(192, 168, 1, 105));
    addr.m_port = userlib_t::htons(TEST_TCP_PORT);

    int ret = 0;
    if ((ret = userlib_t::connect(sock_fd, &addr)) < 0) {
        userlib_t::printf("client connect to fd: %u failed, error %u\n", sock_fd, -ret);
        return;
    }

    userlib_t::printf("client connect success\n");
    char buffer[512] = {0};
    for (int i = 0; i < 1; i++) {
        userlib_t::memset(buffer, 0, 512);
        userlib_t::printf("input some data: ");
        userlib_t::gets(buffer, 512);

        ret = userlib_t::write(sock_fd, buffer, userlib_t::strlen(buffer));
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to write, error %u\n", ret);
            break;
        }
        userlib_t::printf("client write %d bytes to server.\n", ret);

        ret = userlib_t::read(sock_fd, buffer, ret);
        if (ret < 0) {
            userlib_t::printf("ERROR, failed to read, error %u\n", ret);
            break;
        }
        userlib_t::printf("client read %d bytes from server: %s\n", ret, buffer);
    }
    userlib_t::close(sock_fd);
}

static void test_tcp_client()
{
    int32 pid = userlib_t::fork();
    if (pid == 0) {
        /* server */
        tcp_client();
        userlib_t::exit(0);
    }

    /* shell */
    userlib_t::wait(pid);
}

static void test_tcp_server()
{
    int32 pid = userlib_t::fork();
    if (pid == 0) {
        /* server */
        tcp_server();
        userlib_t::exit(0);
    }

    /* shell */
    userlib_t::wait(pid);
}

/************************ tcp ************************/

int main()
{
    char cmd_line[MAX_CMD_LEN] = {0};
    userlib_t::printf("This is printed by shell.\n");

    while (true) {
        userlib_t::color_print(GREEN, "liuruyi $ ");
        userlib_t::gets(cmd_line, MAX_CMD_LEN);
        if (userlib_t::strlen(cmd_line) == 0) {
            continue;
        }

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
        if (userlib_t::strncmp(cmd_line, "test2 ", 6) == 0) {
            test_fork_wait_exit(cmd_line + 6);
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
        if (userlib_t::strncmp(cmd_line, "testudpserver", 13) == 0) {
            test_udp_server();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "testudpclient", 13) == 0) {
            test_udp_client();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "nslookup", 8) == 0) {
            ns_lookup(cmd_line + 9);
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "tcpserver", 9) == 0) {
            test_tcp_server();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "tcpclient", 9) == 0) {
            test_tcp_client();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "ts", 2) == 0) {
            test_tcp_server();
            continue;
        }
        if (userlib_t::strncmp(cmd_line, "tc", 2) == 0) {
            test_tcp_client();
            continue;
        }

        do_cmd(cmd_line);
    }

    userlib_t::exit(0);
    return 1;
}

