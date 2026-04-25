/************************************************************************
 * OSAL - socket系统调用封装实现（Linux）
 ************************************************************************/

#include "net/osal_socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>

/*===========================================================================
 * Socket基本操作
 *===========================================================================*/

int32 OSAL_socket(int32 domain, int32 type, int32 protocol)
{
    return (int32)socket(domain, type, protocol);
}

int32 OSAL_bind(int32 sockfd, const osal_sockaddr_t *addr, osal_size_t addrlen)
{
    return (int32)bind(sockfd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

int32 OSAL_listen(int32 sockfd, int32 backlog)
{
    return (int32)listen(sockfd, backlog);
}

int32 OSAL_accept(int32 sockfd, osal_sockaddr_t *addr, osal_size_t *addrlen)
{
    socklen_t len = addrlen ? (socklen_t)(*addrlen) : 0;
    int32 result = (int32)accept(sockfd, (struct sockaddr *)addr, addrlen ? &len : NULL);
    if (addrlen) {
        *addrlen = (osal_size_t)len;
    }
    return result;
}

int32 OSAL_connect(int32 sockfd, const osal_sockaddr_t *addr, osal_size_t addrlen)
{
    return (int32)connect(sockfd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

osal_ssize_t OSAL_send(int32 sockfd, const void *buf, osal_size_t len, int32 flags)
{
    return (osal_ssize_t)send(sockfd, buf, (size_t)len, flags);
}

osal_ssize_t OSAL_recv(int32 sockfd, void *buf, osal_size_t len, int32 flags)
{
    return (osal_ssize_t)recv(sockfd, buf, (size_t)len, flags);
}

osal_ssize_t OSAL_sendto(int32 sockfd, const void *buf, osal_size_t len, int32 flags,
                  const osal_sockaddr_t *dest_addr, osal_size_t addrlen)
{
    return (osal_ssize_t)sendto(sockfd, buf, (size_t)len, flags,
                         (const struct sockaddr *)dest_addr, (socklen_t)addrlen);
}

osal_ssize_t OSAL_recvfrom(int32 sockfd, void *buf, osal_size_t len, int32 flags,
                    osal_sockaddr_t *src_addr, osal_size_t *addrlen)
{
    socklen_t slen = addrlen ? (socklen_t)(*addrlen) : 0;
    osal_ssize_t result = (osal_ssize_t)recvfrom(sockfd, buf, (size_t)len, flags,
                                   (struct sockaddr *)src_addr, addrlen ? &slen : NULL);
    if (addrlen) {
        *addrlen = (osal_size_t)slen;
    }
    return result;
}

int32 OSAL_shutdown(int32 sockfd, int32 how)
{
    return (int32)shutdown(sockfd, how);
}

/*===========================================================================
 * Socket选项操作
 *===========================================================================*/

int32 OSAL_setsockopt(int32 sockfd, int32 level, int32 optname,
                      const void *optval, osal_size_t optlen)
{
    return (int32)setsockopt(sockfd, level, optname, optval, (socklen_t)optlen);
}

int32 OSAL_getsockopt(int32 sockfd, int32 level, int32 optname,
                      void *optval, osal_size_t *optlen)
{
    socklen_t len = (socklen_t)(*optlen);
    int32 result = (int32)getsockopt(sockfd, level, optname, optval, &len);
    *optlen = (osal_size_t)len;
    return result;
}

/*===========================================================================
 * 网络接口操作
 *===========================================================================*/

uint32 OSAL_if_nametoindex(const char *ifname)
{
    return (uint32)if_nametoindex(ifname);
}

char *OSAL_if_indextoname(uint32 ifindex, char *ifname)
{
    return if_indextoname((unsigned int)ifindex, ifname);
}

/*===========================================================================
 * 字节序转换
 *===========================================================================*/

uint16 OSAL_htons(uint16 hostshort)
{
    return htons(hostshort);
}

uint32 OSAL_htonl(uint32 hostlong)
{
    return htonl(hostlong);
}

uint16 OSAL_ntohs(uint16 netshort)
{
    return ntohs(netshort);
}

uint32 OSAL_ntohl(uint32 netlong)
{
    return ntohl(netlong);
}

/*===========================================================================
 * IP地址转换
 *===========================================================================*/

int32 OSAL_inet_pton(int32 af, const char *src, void *dst)
{
    return (int32)inet_pton(af, src, dst);
}

const char *OSAL_inet_ntop(int32 af, const void *src, char *dst, osal_size_t size)
{
    return inet_ntop(af, src, dst, (socklen_t)size);
}
