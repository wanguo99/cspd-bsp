/************************************************************************
 * 文件I/O API
 ************************************************************************/

#ifndef OSAPI_FILE_H
#define OSAPI_FILE_H

#include "common_types.h"

/*
 * 文件打开标志
 */
#define OS_FILE_FLAG_NONE       0x0000
#define OS_FILE_FLAG_CREATE     0x0001  /* O_CREAT */
#define OS_FILE_FLAG_TRUNCATE   0x0002  /* O_TRUNC */
#define OS_FILE_FLAG_APPEND     0x0004  /* O_APPEND */
#define OS_FILE_FLAG_NONBLOCK   0x0008  /* O_NONBLOCK */

/*
 * 文件访问模式
 */
#define OS_FILE_MODE_NONE       0
#define OS_FILE_MODE_READ       1  /* O_RDONLY */
#define OS_FILE_MODE_WRITE      2  /* O_WRONLY */
#define OS_FILE_MODE_RDWR       3  /* O_RDWR */

/*
 * 文件定位方式
 */
#define OS_FILE_SEEK_SET        0  /* SEEK_SET */
#define OS_FILE_SEEK_CUR        1  /* SEEK_CUR */
#define OS_FILE_SEEK_END        2  /* SEEK_END */

/*
 * ioctl请求码 (保持原始值，由平台层转换)
 */
typedef uint32 os_ioctl_req_t;

/**
 * @brief 打开文件
 *
 * @param[out] fd       文件描述符ID
 * @param[in]  path     文件路径
 * @param[in]  mode     访问模式 (OS_FILE_MODE_*)
 * @param[in]  flags    打开标志 (OS_FILE_FLAG_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_INVALID_POINTER fd或path为NULL
 * @return OS_ERROR 打开失败
 */
int32 OS_FileOpen(osal_id_t *fd, const char *path, uint32 mode, uint32 flags);

/**
 * @brief 关闭文件
 *
 * @param[in] fd 文件描述符ID
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 */
int32 OS_FileClose(osal_id_t fd);

/**
 * @brief 读取文件
 *
 * @param[in]  fd      文件描述符ID
 * @param[out] buffer  读取缓冲区
 * @param[in]  size    要读取的字节数
 *
 * @return 实际读取的字节数(>=0)
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR 读取失败
 */
int32 OS_FileRead(osal_id_t fd, void *buffer, uint32 size);

/**
 * @brief 写入文件
 *
 * @param[in] fd     文件描述符ID
 * @param[in] buffer 写入缓冲区
 * @param[in] size   要写入的字节数
 *
 * @return 实际写入的字节数(>=0)
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR 写入失败
 */
int32 OS_FileWrite(osal_id_t fd, const void *buffer, uint32 size);

/**
 * @brief 文件定位
 *
 * @param[in] fd     文件描述符ID
 * @param[in] offset 偏移量
 * @param[in] whence 定位方式 (OS_FILE_SEEK_*)
 *
 * @return 新的文件位置(>=0)
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR 定位失败
 */
int32 OS_FileSeek(osal_id_t fd, int32 offset, uint32 whence);

/**
 * @brief 设置文件控制标志
 *
 * @param[in] fd    文件描述符ID
 * @param[in] flags 标志 (OS_FILE_FLAG_*)
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR 设置失败
 */
int32 OS_FileSetFlags(osal_id_t fd, uint32 flags);

/**
 * @brief 获取文件控制标志
 *
 * @param[in]  fd    文件描述符ID
 * @param[out] flags 标志
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR 获取失败
 */
int32 OS_FileGetFlags(osal_id_t fd, uint32 *flags);

/**
 * @brief 文件I/O控制
 *
 * @param[in]    fd      文件描述符ID
 * @param[in]    request ioctl请求码
 * @param[inout] arg     参数指针
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERR_INVALID_ID 无效的文件描述符
 * @return OS_ERROR ioctl失败
 */
int32 OS_FileIoctl(osal_id_t fd, os_ioctl_req_t request, void *arg);

/**
 * @brief 等待文件I/O就绪
 *
 * @param[in]    fds        文件描述符数组
 * @param[in]    nfds       文件描述符数量
 * @param[inout] read_set   可读集合 (位掩码，输入时设置要检查的fd，输出时返回就绪的fd)
 * @param[inout] write_set  可写集合 (位掩码)
 * @param[inout] error_set  错误集合 (位掩码)
 * @param[in]    timeout_ms 超时时间(毫秒), OS_PEND表示阻塞, 0表示立即返回
 *
 * @return 就绪的文件描述符数量(>=0)
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR select失败
 */
int32 OS_FileSelect(const osal_id_t *fds, uint32 nfds,
                    uint32 *read_set, uint32 *write_set, uint32 *error_set,
                    int32 timeout_ms);

#endif /* OSAPI_FILE_H */
