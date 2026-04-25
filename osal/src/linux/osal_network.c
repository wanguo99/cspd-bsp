/************************************************************************
 * 网络通信实现 (Linux)
 ************************************************************************/

#include "osal.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_SOCKETS  64

typedef struct
{
    bool   in_use;
    int32  native_fd;
    uint32 type;
} socket_record_t;

static socket_record_t g_socket_table[MAX_SOCKETS];
static osal_id_t       g_socket_mutex = OS_OBJECT_ID_UNDEFINED;
static pthread_mutex_t g_socket_init_mutex = PTHREAD_MUTEX_INITIALIZER;

int32 OSAL_SocketOpen(osal_id_t *sock_id, uint32 domain, uint32 type)
{
    int32 native_fd;
    int32 native_domain;
    int32 native_type;
    uint32 i;

    if (sock_id == NULL)
        return OS_INVALID_POINTER;

    /* 转换domain */
    switch (domain)
    {
        case OS_SOCK_DOMAIN_INET:
            native_domain = AF_INET;
            break;
        case OS_SOCK_DOMAIN_INET6:
            native_domain = AF_INET6;
            break;
        default:
            return OS_ERROR;
    }

    switch (type)
    {
        case OS_SOCK_STREAM:
            native_type = SOCK_STREAM;
            break;
        case OS_SOCK_DGRAM:
            native_type = SOCK_DGRAM;
            break;
        default:
            return OS_ERROR;
    }

    native_fd = socket(native_domain, native_type, 0);
    if (native_fd < 0)
    {
        OS_printf("[OSAL] 创建socket失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    /* 线程安全的延迟初始化 */
    if (g_socket_mutex == OS_OBJECT_ID_UNDEFINED)
    {
        pthread_mutex_lock(&g_socket_init_mutex);
        if (g_socket_mutex == OS_OBJECT_ID_UNDEFINED)
        {
            OSAL_MutexCreate(&g_socket_mutex, "SOCK_MTX", 0);
        }
        pthread_mutex_unlock(&g_socket_init_mutex);
    }

    OSAL_MutexLock(g_socket_mutex);

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (!g_socket_table[i].in_use)
        {
            g_socket_table[i].in_use = true;
            g_socket_table[i].native_fd = native_fd;
            g_socket_table[i].type = type;
            *sock_id = i + 1;
            OSAL_MutexUnlock(g_socket_mutex);
            return OS_SUCCESS;
        }
    }

    OSAL_MutexUnlock(g_socket_mutex);
    close(native_fd);
    return OS_ERR_NO_FREE_IDS;
}

int32 OSAL_SocketClose(osal_id_t sock_id)
{
    uint32 index;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    close(g_socket_table[index].native_fd);
    g_socket_table[index].in_use = false;
    g_socket_table[index].native_fd = -1;

    OSAL_MutexUnlock(g_socket_mutex);
    return OS_SUCCESS;
}

int32 OSAL_SocketBind(osal_id_t sock_id, const char *addr, uint16 port)
{
    uint32 index;
    int32 native_fd;
    struct sockaddr_in sockaddr;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    if (addr == NULL || addr[0] == '\0')
        sockaddr.sin_addr.s_addr = INADDR_ANY;
    else
        sockaddr.sin_addr.s_addr = inet_addr(addr);

    if (bind(native_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        OS_printf("[OSAL] bind失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SocketConnect(osal_id_t sock_id, const char *addr, uint16 port, int32 timeout)
{
    uint32 index;
    int32 native_fd;
    struct sockaddr_in sockaddr;
    int32 flags;
    int32 result;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (addr == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(addr);

    if (timeout != OS_PEND)
    {
        flags = fcntl(native_fd, F_GETFL, 0);
        if (flags < 0)
        {
            OS_printf("[OSAL] fcntl(F_GETFL) 失败, errno=%d\n", errno);
            return OS_ERROR;
        }

        if (fcntl(native_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            OS_printf("[OSAL] fcntl(F_SETFL) 失败, errno=%d\n", errno);
            return OS_ERROR;
        }
    }

    result = connect(native_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));

    if (result < 0 && errno == EINPROGRESS && timeout != OS_PEND)
    {
        fd_set writefds;
        struct timeval tv;
        int32 select_ret;
        int32 error;
        socklen_t error_len = sizeof(error);

        FD_ZERO(&writefds);
        FD_SET(native_fd, &writefds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select_ret = select(native_fd + 1, NULL, &writefds, NULL, &tv);

        if (select_ret < 0)
        {
            fcntl(native_fd, F_SETFL, flags);
            return OS_ERROR;
        }

        if (select_ret == 0)
        {
            fcntl(native_fd, F_SETFL, flags);
            return OS_ERROR_TIMEOUT;
        }

        if (getsockopt(native_fd, SOL_SOCKET, SO_ERROR, &error, &error_len) < 0)
        {
            fcntl(native_fd, F_SETFL, flags);
            return OS_ERROR;
        }

        if (error != 0)
        {
            fcntl(native_fd, F_SETFL, flags);
            return OS_ERROR;
        }

        fcntl(native_fd, F_SETFL, flags);
        return OS_SUCCESS;
    }

    if (timeout != OS_PEND)
    {
        fcntl(native_fd, F_SETFL, flags);
    }

    if (result < 0)
    {
        OS_printf("[OSAL] connect失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SocketListen(osal_id_t sock_id, uint32 backlog)
{
    uint32 index;
    int32 native_fd;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    if (listen(native_fd, (int)backlog) < 0)
    {
        OS_printf("[OSAL] listen失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SocketAccept(osal_id_t sock_id, osal_id_t *conn_id, char *addr, int32 timeout)
{
    uint32 index;
    int32 native_fd;
    int32 conn_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint32 i;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (conn_id == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    if (timeout != OS_PEND)
    {
        fd_set readfds;
        struct timeval tv;
        int32 select_ret;

        FD_ZERO(&readfds);
        FD_SET(native_fd, &readfds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select_ret = select(native_fd + 1, &readfds, NULL, NULL, &tv);
        if (select_ret < 0)
            return OS_ERROR;
        if (select_ret == 0)
            return OS_ERROR_TIMEOUT;
    }

    conn_fd = accept(native_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (conn_fd < 0)
    {
        OS_printf("[OSAL] accept失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    if (addr != NULL)
    {
        /* 使用线程安全的 inet_ntop */
        if (inet_ntop(AF_INET, &client_addr.sin_addr, addr, 16) == NULL)
        {
            addr[0] = '\0';
        }
    }

    OSAL_MutexLock(g_socket_mutex);

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if (!g_socket_table[i].in_use)
        {
            g_socket_table[i].in_use = true;
            g_socket_table[i].native_fd = conn_fd;
            g_socket_table[i].type = OS_SOCK_STREAM;
            *conn_id = i + 1;
            OSAL_MutexUnlock(g_socket_mutex);
            return OS_SUCCESS;
        }
    }

    OSAL_MutexUnlock(g_socket_mutex);
    close(conn_fd);
    return OS_ERR_NO_FREE_IDS;
}

int32 OSAL_SocketSend(osal_id_t sock_id, const void *buffer, uint32 size, int32 timeout)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_sent;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    if (timeout != OS_PEND)
    {
        fd_set writefds;
        struct timeval tv;
        int32 select_ret;

        FD_ZERO(&writefds);
        FD_SET(native_fd, &writefds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select_ret = select(native_fd + 1, NULL, &writefds, NULL, &tv);
        if (select_ret < 0)
            return OS_ERROR;
        if (select_ret == 0)
            return OS_ERROR_TIMEOUT;
    }

    bytes_sent = send(native_fd, buffer, (size_t)size, MSG_NOSIGNAL);
    if (bytes_sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    return bytes_sent;
}

int32 OSAL_SocketRecv(osal_id_t sock_id, void *buffer, uint32 size, int32 timeout)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_recv;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    if (timeout != OS_PEND)
    {
        fd_set readfds;
        struct timeval tv;
        int32 select_ret;

        FD_ZERO(&readfds);
        FD_SET(native_fd, &readfds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select_ret = select(native_fd + 1, &readfds, NULL, NULL, &tv);
        if (select_ret < 0)
            return OS_ERROR;
        if (select_ret == 0)
            return OS_ERROR_TIMEOUT;
    }

    bytes_recv = recv(native_fd, buffer, (size_t)size, 0);
    if (bytes_recv < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    return bytes_recv;
}

int32 OSAL_SocketSendTo(osal_id_t sock_id, const void *buffer, uint32 size,
                      const char *addr, uint16 port)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_sent;
    struct sockaddr_in dest_addr;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL || addr == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(addr);

    bytes_sent = sendto(native_fd, buffer, (size_t)size, 0,
                        (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (bytes_sent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    return bytes_sent;
}

int32 OSAL_SocketRecvFrom(osal_id_t sock_id, void *buffer, uint32 size,
                        char *addr, uint16 *port, int32 timeout)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_recv;
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    if (timeout != OS_PEND)
    {
        fd_set readfds;
        struct timeval tv;
        int32 select_ret;

        FD_ZERO(&readfds);
        FD_SET(native_fd, &readfds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        select_ret = select(native_fd + 1, &readfds, NULL, NULL, &tv);
        if (select_ret < 0)
            return OS_ERROR;
        if (select_ret == 0)
            return OS_ERROR_TIMEOUT;
    }

    bytes_recv = recvfrom(native_fd, buffer, (size_t)size, 0,
                          (struct sockaddr *)&src_addr, &addr_len);
    if (bytes_recv < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    if (addr != NULL)
    {
        /* 使用线程安全的 inet_ntop */
        if (inet_ntop(AF_INET, &src_addr.sin_addr, addr, 16) == NULL)
        {
            addr[0] = '\0';
        }
    }

    if (port != NULL)
    {
        *port = ntohs(src_addr.sin_port);
    }

    return bytes_recv;
}

int32 OSAL_SocketSetOpt(osal_id_t sock_id, uint32 level, uint32 optname,
                      const void *optval, uint32 optlen)
{
    uint32 index;
    int32 native_fd;
    int32 native_level;
    int32 native_optname;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (optval == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    switch (level)
    {
        case OS_SOL_SOCKET:
            native_level = SOL_SOCKET;
            break;
        case OS_SOL_TCP:
            native_level = IPPROTO_TCP;
            break;
        case OS_SOL_IP:
            native_level = IPPROTO_IP;
            break;
        default:
            return OS_ERROR;
    }

    switch (optname)
    {
        case OS_SO_REUSEADDR:
            native_optname = SO_REUSEADDR;
            break;
        case OS_SO_KEEPALIVE:
            native_optname = SO_KEEPALIVE;
            break;
        case OS_SO_RCVTIMEO:
            native_optname = SO_RCVTIMEO;
            break;
        case OS_SO_SNDTIMEO:
            native_optname = SO_SNDTIMEO;
            break;
        case OS_SO_RCVBUF:
            native_optname = SO_RCVBUF;
            break;
        case OS_SO_SNDBUF:
            native_optname = SO_SNDBUF;
            break;
        case OS_SO_ERROR:
            native_optname = SO_ERROR;
            break;
        case OS_TCP_NODELAY:
            native_optname = TCP_NODELAY;
            break;
        default:
            return OS_ERROR;
    }

    if (setsockopt(native_fd, native_level, native_optname, optval, (socklen_t)optlen) < 0)
    {
        OS_printf("[OSAL] setsockopt失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 OSAL_SocketGetOpt(osal_id_t sock_id, uint32 level, uint32 optname,
                      void *optval, uint32 *optlen)
{
    uint32 index;
    int32 native_fd;
    int32 native_level;
    int32 native_optname;
    socklen_t native_optlen;

    if (sock_id == 0 || sock_id > MAX_SOCKETS)
        return OS_ERR_INVALID_ID;

    if (optval == NULL || optlen == NULL)
        return OS_INVALID_POINTER;

    index = sock_id - 1;

    OSAL_MutexLock(g_socket_mutex);

    if (!g_socket_table[index].in_use)
    {
        OSAL_MutexUnlock(g_socket_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_socket_table[index].native_fd;
    OSAL_MutexUnlock(g_socket_mutex);

    switch (level)
    {
        case OS_SOL_SOCKET:
            native_level = SOL_SOCKET;
            break;
        case OS_SOL_TCP:
            native_level = IPPROTO_TCP;
            break;
        case OS_SOL_IP:
            native_level = IPPROTO_IP;
            break;
        default:
            return OS_ERROR;
    }

    switch (optname)
    {
        case OS_SO_REUSEADDR:
            native_optname = SO_REUSEADDR;
            break;
        case OS_SO_KEEPALIVE:
            native_optname = SO_KEEPALIVE;
            break;
        case OS_SO_RCVTIMEO:
            native_optname = SO_RCVTIMEO;
            break;
        case OS_SO_SNDTIMEO:
            native_optname = SO_SNDTIMEO;
            break;
        case OS_SO_RCVBUF:
            native_optname = SO_RCVBUF;
            break;
        case OS_SO_SNDBUF:
            native_optname = SO_SNDBUF;
            break;
        case OS_SO_ERROR:
            native_optname = SO_ERROR;
            break;
        case OS_TCP_NODELAY:
            native_optname = TCP_NODELAY;
            break;
        default:
            return OS_ERROR;
    }

    native_optlen = (socklen_t)*optlen;

    if (getsockopt(native_fd, native_level, native_optname, optval, &native_optlen) < 0)
    {
        OS_printf("[OSAL] getsockopt失败, errno=%d\n", errno);
        return OS_ERROR;
    }

    *optlen = (uint32)native_optlen;
    return OS_SUCCESS;
}

int32 OSAL_SocketSelect(const osal_id_t *socks, uint32 nsocks,
                      uint32 *read_set, uint32 *write_set, uint32 *error_set,
                      int32 timeout_ms)
{
    fd_set readfds, writefds, errorfds;
    struct timeval tv;
    struct timeval *tv_ptr;
    int32 max_fd = -1;
    int32 result;
    uint32 i;

    if (socks == NULL || nsocks == 0 || nsocks > 32)
        return OS_ERROR;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);

    OSAL_MutexLock(g_socket_mutex);

    for (i = 0; i < nsocks; i++)
    {
        uint32 index;
        int32 native_fd;

        if (socks[i] == 0 || socks[i] > MAX_SOCKETS)
            continue;

        index = socks[i] - 1;

        if (!g_socket_table[index].in_use)
            continue;

        native_fd = g_socket_table[index].native_fd;

        if (read_set && (*read_set & (1U << i)))
            FD_SET(native_fd, &readfds);

        if (write_set && (*write_set & (1U << i)))
            FD_SET(native_fd, &writefds);

        if (error_set && (*error_set & (1U << i)))
            FD_SET(native_fd, &errorfds);

        if (native_fd > max_fd)
            max_fd = native_fd;
    }

    OSAL_MutexUnlock(g_socket_mutex);

    if (max_fd < 0)
        return OS_ERROR;

    if (timeout_ms == OS_PEND)
    {
        tv_ptr = NULL;
    }
    else
    {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tv_ptr = &tv;
    }

    result = select(max_fd + 1, &readfds, &writefds, &errorfds, tv_ptr);

    if (result < 0)
    {
        if (errno == EINTR)
            return OS_ERR_INTERRUPTED;
        return OS_ERROR;
    }

    if (result == 0)
        return OS_ERROR_TIMEOUT;

    if (read_set)
        *read_set = 0;
    if (write_set)
        *write_set = 0;
    if (error_set)
        *error_set = 0;

    OSAL_MutexLock(g_socket_mutex);

    for (i = 0; i < nsocks; i++)
    {
        uint32 index;
        int32 native_fd;

        if (socks[i] == 0 || socks[i] > MAX_SOCKETS)
            continue;

        index = socks[i] - 1;

        if (!g_socket_table[index].in_use)
            continue;

        native_fd = g_socket_table[index].native_fd;

        if (read_set && FD_ISSET(native_fd, &readfds))
            *read_set |= (1U << i);

        if (write_set && FD_ISSET(native_fd, &writefds))
            *write_set |= (1U << i);

        if (error_set && FD_ISSET(native_fd, &errorfds))
            *error_set |= (1U << i);
    }

    OSAL_MutexUnlock(g_socket_mutex);

    return result;
}
