/************************************************************************
 * 网络通信API
 ************************************************************************/

#ifndef OSAPI_NETWORK_H
#define OSAPI_NETWORK_H

#include "common_types.h"

/*
 * Socket类型
 */
#define OS_SOCK_STREAM  1  /* TCP */
#define OS_SOCK_DGRAM   2  /* UDP */

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
int32 OS_SocketOpen(osal_id_t *sock_id, uint32 domain, uint32 type);

/**
 * @brief 关闭Socket
 *
 * @param[in] sock_id Socket ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的Socket ID
 */
int32 OS_SocketClose(osal_id_t sock_id);

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
int32 OS_SocketBind(osal_id_t sock_id, const char *addr, uint16 port);

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
int32 OS_SocketConnect(osal_id_t sock_id, const char *addr, uint16 port, int32 timeout);

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
int32 OS_SocketListen(osal_id_t sock_id, uint32 backlog);

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
int32 OS_SocketAccept(osal_id_t sock_id, osal_id_t *conn_id, char *addr, int32 timeout);

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
int32 OS_SocketSend(osal_id_t sock_id, const void *buffer, uint32 size, int32 timeout);

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
int32 OS_SocketRecv(osal_id_t sock_id, void *buffer, uint32 size, int32 timeout);

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
int32 OS_SocketSendTo(osal_id_t sock_id, const void *buffer, uint32 size,
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
int32 OS_SocketRecvFrom(osal_id_t sock_id, void *buffer, uint32 size,
                        char *addr, uint16 *port, int32 timeout);

#endif /* OSAPI_NETWORK_H */
