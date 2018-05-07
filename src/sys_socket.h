/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _SYS_SOCKET_H_
#define _SYS_SOCKET_H_

#include "types.h"
#include "traps.h"
#include "spinlock.h"
#include "socket.h"

class sys_socket_t {
public:
    enum socket_call_e {
        SOCK_SOCKET = 0,
        SOCK_BIND,
        SOCK_LISTEN,
        SOCK_ACCEPT,
        SOCK_CONNECT,
        MAX_SYS_SOCKET,
    };

    static void      init();
    static int32     do_sys_socket(trap_frame_t* frame);
    //static int32     close_socket(socket_t* socket);

private:
    static int32     sys_socket(trap_frame_t* frame);
    static int32     sys_bind(trap_frame_t* frame);
    static int32     sys_listen(trap_frame_t* frame);
    static int32     sys_accept(trap_frame_t* frame);
    static int32     sys_connect(trap_frame_t* frame);

    static int32     socket(uint32 family, uint32 type, uint32 protocol);
    static int32     bind(int fd, sock_addr_t* myaddr);
    static int32     listen(int fd, uint32 backlog);
    static int32     accept(int fd, sock_addr_t* peer_addr);
    static int32     connect(int fd, sock_addr_t* user_addr);

    static socket_t* alloc_socket(uint32 family, uint32 type);
    static int32     release_socket(socket_t* socket);

    static socket_t* look_up_socket(int fd);

private:
    static int32 (*s_sys_socket_table[])(trap_frame_t* frame);
};

#endif

