/************************************************************************
 * 网络通信API
 ************************************************************************/

#ifndef OSAPI_NETWORK_H
#define OSAPI_NETWORK_H

#include "osal_types.h"

/*
 * Socket类型
 */
#define OS_SOCK_STREAM  1  /* TCP */
#define OS_SOCK_DGRAM   2  /* UDP */

/*
 * Socket协议族
 */
#define OS_SOCK_DOMAIN_INET   1  /* IPv4 */
#define OS_SOCK_DOMAIN_INET6  2  /* IPv6 */

/**
 * @brief 创建Socket
 *
 * @param[out] sock_id Socket ID
 * @param[in]  domain  协议族(保留,传0)
 * @param[in]  type    Socket类型(OS_SOCK_STREAM或OS_SOCK_DGRAM)
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER sock_id为NULL
 * @return OS_ERROR 创建失败
 */
int32 OSAL_SocketOpen(osal_id_t *sock_id, uint32 domain, uint32 type);

/**
 * @brief 关闭Socket
 *
 * @param[in] sock_id Socket ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 */
int32 OSAL_SocketClose(osal_id_t sock_id);

/**
 * @brief 绑定Socket到地址
 *
 * @param[in] sock_id Socket ID
 * @param[in] addr    IP地址字符串(例如"192.168.1.100")
 * @param[in] port    端口号
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_ERROR 绑定失败
 */
int32 OSAL_SocketBind(osal_id_t sock_id, const char *addr, uint16 port);

/**
 * @brief 连接到远程地址(TCP)
 *
 * @param[in] sock_id Socket ID
 * @param[in] addr    远程IP地址
 * @param[in] port    远程端口
 * @param[in] timeout 超时时间(毫秒), OS_PEND表示阻塞
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_ERROR_TIMEOUT 连接超时
 * @return OS_ERROR 连接失败
 */
int32 OSAL_SocketConnect(osal_id_t sock_id, const char *addr, uint16 port, int32 timeout);

/**
 * @brief 监听连接(TCP)
 *
 * @param[in] sock_id Socket ID
 * @param[in] backlog 等待队列长度
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_ERROR 监听失败
 */
int32 OSAL_SocketListen(osal_id_t sock_id, uint32 backlog);

/**
 * @brief 接受连接(TCP)
 *
 * @param[in]  sock_id     监听Socket ID
 * @param[out] conn_id     新连接的Socket ID
 * @param[out] addr        客户端地址(可为NULL)
 * @param[in]  timeout     超时时间(毫秒)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 接受失败
 */
int32 OSAL_SocketAccept(osal_id_t sock_id, osal_id_t *conn_id, char *addr, int32 timeout);

/**
 * @brief 发送数据
 *
 * @param[in] sock_id Socket ID
 * @param[in] buffer  数据缓冲区
 * @param[in] size    数据大小
 * @param[in] timeout 超时时间(毫秒)
 *
 * @return 实际发送的字节数(>=0)
 * @return OS_ERROR 发送失败
 */
int32 OSAL_SocketSend(osal_id_t sock_id, const void *buffer, uint32 size, int32 timeout);

/**
 * @brief 接收数据
 *
 * @param[in]  sock_id Socket ID
 * @param[out] buffer  接收缓冲区
 * @param[in]  size    缓冲区大小
 * @param[in]  timeout 超时时间(毫秒)
 *
 * @return 实际接收的字节数(>=0)
 * @return OS_ERROR 接收失败
 */
int32 OSAL_SocketRecv(osal_id_t sock_id, void *buffer, uint32 size, int32 timeout);

/**
 * @brief 发送UDP数据报
 *
 * @param[in] sock_id Socket ID
 * @param[in] buffer  数据缓冲区
 * @param[in] size    数据大小
 * @param[in] addr    目标地址
 * @param[in] port    目标端口
 *
 * @return 实际发送的字节数(>=0)
 * @return OS_ERROR 发送失败
 */
int32 OSAL_SocketSendTo(osal_id_t sock_id, const void *buffer, uint32 size,
                      const char *addr, uint16 port);

/**
 * @brief 接收UDP数据报
 *
 * @param[in]  sock_id Socket ID
 * @param[out] buffer  接收缓冲区
 * @param[in]  size    缓冲区大小
 * @param[out] addr    源地址(可为NULL)
 * @param[out] port    源端口(可为NULL)
 * @param[in]  timeout 超时时间(毫秒)
 *
 * @return 实际接收的字节数(>=0)
 * @return OS_ERROR 接收失败
 */
int32 OSAL_SocketRecvFrom(osal_id_t sock_id, void *buffer, uint32 size,
                        char *addr, uint16 *port, int32 timeout);

/*
 * Socket选项级别
 */
#define OS_SOL_SOCKET       1  /* SOL_SOCKET */
#define OS_SOL_TCP          6  /* IPPROTO_TCP */
#define OS_SOL_IP           0  /* IPPROTO_IP */

/*
 * Socket选项名称
 */
#define OS_SO_REUSEADDR     0x0001  /* SO_REUSEADDR */
#define OS_SO_KEEPALIVE     0x0002  /* SO_KEEPALIVE */
#define OS_SO_RCVTIMEO      0x0003  /* SO_RCVTIMEO */
#define OS_SO_SNDTIMEO      0x0004  /* SO_SNDTIMEO */
#define OS_SO_RCVBUF        0x0005  /* SO_RCVBUF */
#define OS_SO_SNDBUF        0x0006  /* SO_SNDBUF */
#define OS_SO_ERROR         0x0007  /* SO_ERROR */
#define OS_TCP_NODELAY      0x0101  /* TCP_NODELAY */

/**
 * @brief 设置Socket选项
 *
 * @param[in] sock_id Socket ID
 * @param[in] level   选项级别 (OS_SOL_*)
 * @param[in] optname 选项名称 (OS_SO_* 或 OS_TCP_*)
 * @param[in] optval  选项值指针
 * @param[in] optlen  选项值长度
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_INVALID_POINTER optval为NULL
 * @return OS_ERROR 设置失败
 */
int32 OSAL_SocketSetOpt(osal_id_t sock_id, uint32 level, uint32 optname,
                      const void *optval, uint32 optlen);

/**
 * @brief 获取Socket选项
 *
 * @param[in]    sock_id Socket ID
 * @param[in]    level   选项级别 (OS_SOL_*)
 * @param[in]    optname 选项名称 (OS_SO_* 或 OS_TCP_*)
 * @param[out]   optval  选项值指针
 * @param[inout] optlen  输入时为缓冲区大小，输出时为实际长度
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 * @return OS_INVALID_POINTER optval或optlen为NULL
 * @return OS_ERROR 获取失败
 */
int32 OSAL_SocketGetOpt(osal_id_t sock_id, uint32 level, uint32 optname,
                      void *optval, uint32 *optlen);

/**
 * @brief 等待Socket I/O就绪
 *
 * @param[in]    socks      Socket ID数组
 * @param[in]    nsocks     Socket数量
 * @param[inout] read_set   可读集合 (位掩码，输入时设置要检查的socket，输出时返回就绪的socket)
 * @param[inout] write_set  可写集合 (位掩码)
 * @param[inout] error_set  错误集合 (位掩码)
 * @param[in]    timeout_ms 超时时间(毫秒), OS_PEND表示阻塞, 0表示立即返回
 *
 * @return 就绪的Socket数量(>=0)
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR select失败
 */
int32 OSAL_SocketSelect(const osal_id_t *socks, uint32 nsocks,
                      uint32 *read_set, uint32 *write_set, uint32 *error_set,
                      int32 timeout_ms);

#endif /* OSAPI_NETWORK_H */
