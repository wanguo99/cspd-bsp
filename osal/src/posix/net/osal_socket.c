/************************************************************************
 * OSAL - socket系统调用封装实现（POSIX）
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

int32_t OSAL_socket(int32_t domain, int32_t type, int32_t protocol)
{
    return (int32_t)socket(domain, type, protocol);
}

int32_t OSAL_bind(int32_t sockfd, const osal_sockaddr_t *addr, osal_size_t addrlen)
{
    return (int32_t)bind(sockfd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

int32_t OSAL_listen(int32_t sockfd, int32_t backlog)
{
    return (int32_t)listen(sockfd, backlog);
}

int32_t OSAL_accept(int32_t sockfd, osal_sockaddr_t *addr, osal_size_t *addrlen)
{
    socklen_t len = addrlen ? (socklen_t)(*addrlen) : 0;
    int32_t result = (int32_t)accept(sockfd, (struct sockaddr *)addr, addrlen ? &len : NULL);
    if (addrlen) {
        *addrlen = (osal_size_t)len;
    }
    return result;
}

int32_t OSAL_connect(int32_t sockfd, const osal_sockaddr_t *addr, osal_size_t addrlen)
{
    return (int32_t)connect(sockfd, (const struct sockaddr *)addr, (socklen_t)addrlen);
}

osal_ssize_t OSAL_send(int32_t sockfd, const void *buf, osal_size_t len, int32_t flags)
{
    return (osal_ssize_t)send(sockfd, buf, (size_t)len, flags);
}

osal_ssize_t OSAL_recv(int32_t sockfd, void *buf, osal_size_t len, int32_t flags)
{
    return (osal_ssize_t)recv(sockfd, buf, (size_t)len, flags);
}

osal_ssize_t OSAL_sendto(int32_t sockfd, const void *buf, osal_size_t len, int32_t flags,
                  const osal_sockaddr_t *dest_addr, osal_size_t addrlen)
{
    return (osal_ssize_t)sendto(sockfd, buf, (size_t)len, flags,
                         (const struct sockaddr *)dest_addr, (socklen_t)addrlen);
}

osal_ssize_t OSAL_recvfrom(int32_t sockfd, void *buf, osal_size_t len, int32_t flags,
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

int32_t OSAL_shutdown(int32_t sockfd, int32_t how)
{
    return (int32_t)shutdown(sockfd, how);
}

/*===========================================================================
 * Socket选项操作
 *===========================================================================*/

int32_t OSAL_setsockopt(int32_t sockfd, int32_t level, int32_t optname,
                      const void *optval, osal_size_t optlen)
{
    return (int32_t)setsockopt(sockfd, level, optname, optval, (socklen_t)optlen);
}

int32_t OSAL_getsockopt(int32_t sockfd, int32_t level, int32_t optname,
                      void *optval, osal_size_t *optlen)
{
    socklen_t len = (socklen_t)(*optlen);
    int32_t result = (int32_t)getsockopt(sockfd, level, optname, optval, &len);
    *optlen = (osal_size_t)len;
    return result;
}

/*===========================================================================
 * 网络接口操作
 *===========================================================================*/

uint32_t OSAL_if_nametoindex(const char *ifname)
{
    return (uint32_t)if_nametoindex(ifname);
}

str_t *OSAL_if_indextoname(uint32_t ifindex, str_t *ifname)
{
    return (str_t *)if_indextoname((unsigned int)ifindex, ifname);
}

/*===========================================================================
 * 字节序转换
 *===========================================================================*/

uint16_t OSAL_htons(uint16_t hostshort)
{
    return htons(hostshort);
}

uint32_t OSAL_htonl(uint32_t hostlong)
{
    return htonl(hostlong);
}

uint16_t OSAL_ntohs(uint16_t netshort)
{
    return ntohs(netshort);
}

uint32_t OSAL_ntohl(uint32_t netlong)
{
    return ntohl(netlong);
}

/*===========================================================================
 * IP地址转换
 *===========================================================================*/

int32_t OSAL_inet_pton(int32_t af, const char *src, void *dst)
{
    return (int32_t)inet_pton(af, src, dst);
}

const char *OSAL_inet_ntop(int32_t af, const void *src, char *dst, osal_size_t size)
{
    return inet_ntop(af, src, dst, (socklen_t)size);
}
