/************************************************************************
 * HAL层 - CAN驱动Linux实现
 *
 * 使用SocketCAN实现CAN通信
 ************************************************************************/

/* 必须在所有头文件之前定义，以启用完整的POSIX功能 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "hal_can.h"
#include "osal.h"

/* 兼容性定义 */
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE
#endif

/*
 * CAN句柄结构
 */
typedef struct
{
    int sockfd;
    uint32 tx_count;
    uint32 rx_count;
    uint32 err_count;
} can_handle_impl_t;

/**
 * @brief 初始化CAN驱动
 */
int32 HAL_CAN_Init(const hal_can_config_t *config __attribute__((unused)),
                   hal_can_handle_t *handle __attribute__((unused)))
{
    can_handle_impl_t *impl;
    struct sockaddr_can addr;

    struct ifreq ifr;
    int ret;

    if (config == NULL || handle == NULL)
        return OS_INVALID_POINTER;

    /* 分配句柄 */
    impl = malloc(sizeof(can_handle_impl_t));
    if (impl == NULL)
        return OS_ERROR;

    memset(impl, 0, sizeof(can_handle_impl_t));

    /* 创建SocketCAN */
    impl->sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (impl->sockfd < 0)
    {
        OS_printf("[HAL CAN] 创建socket失败: %s\n", strerror(errno));
        free(impl);
        return OS_ERROR;
    }

    /* 获取接口索引 */
    strncpy(ifr.ifr_name, config->interface, IFNAMSIZ - 1);
    ret = ioctl(impl->sockfd, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        OS_printf("[HAL CAN] 获取接口索引失败: %s\n", strerror(errno));
        close(impl->sockfd);
        free(impl);
        return OS_ERROR;
    }

    /* 绑定到CAN接口 */
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    ret = bind(impl->sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        OS_printf("[HAL CAN] 绑定接口失败: %s\n", strerror(errno));
        close(impl->sockfd);
        free(impl);
        return OS_ERROR;
    }

    /* 设置接收超时 */
    struct timeval tv;
    tv.tv_sec = config->rx_timeout / 1000;
    tv.tv_usec = (config->rx_timeout % 1000) * 1000;
    setsockopt(impl->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    *handle = (hal_can_handle_t)impl;

    OS_printf("[HAL CAN] 初始化成功: %s\n", config->interface);

    return OS_SUCCESS;
}

/**
 * @brief 关闭CAN驱动
 */
int32 HAL_CAN_Deinit(hal_can_handle_t handle)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    close(impl->sockfd);
    free(impl);

    return OS_SUCCESS;
}

/**
 * @brief 发送CAN帧
 */
int32 HAL_CAN_Send(hal_can_handle_t handle, const can_frame_t *frame)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;
    struct can_frame can_frame;
    int ret;

    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    /* 转换为SocketCAN格式 */
    can_frame.can_id = frame->can_id;
    can_frame.can_dlc = frame->dlc;
    memcpy(can_frame.data, &frame->msg, frame->dlc);

    /* 发送 */
    ret = write(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret != sizeof(struct can_frame))
    {
        impl->err_count++;
        return OS_ERROR;
    }

    impl->tx_count++;
    return OS_SUCCESS;
}

/**
 * @brief 接收CAN帧
 */
int32 HAL_CAN_Recv(hal_can_handle_t handle, can_frame_t *frame, int32 timeout)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;
    struct can_frame can_frame;
    int ret;

    (void)timeout;  /* 超时通过setsockopt设置，此参数暂未使用 */

    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    /* 接收 */
    ret = read(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERROR_TIMEOUT;

        impl->err_count++;
        return OS_ERROR;
    }

    /* 转换为内部格式 */
    frame->can_id = can_frame.can_id;
    frame->dlc = can_frame.can_dlc;
    memcpy(&frame->msg, can_frame.data, can_frame.can_dlc);
    frame->timestamp = OS_GetTickCount();

    impl->rx_count++;
    return OS_SUCCESS;
}

/**
 * @brief 设置CAN过滤器
 */
int32 HAL_CAN_SetFilter(hal_can_handle_t handle, uint32 filter_id, uint32 filter_mask)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;
    struct can_filter rfilter[1];

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    rfilter[0].can_id = filter_id;
    rfilter[0].can_mask = filter_mask;

    if (setsockopt(impl->sockfd, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &rfilter, sizeof(rfilter)) < 0)
    {
        OS_printf("[HAL CAN] 设置过滤器失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/**
 * @brief 获取CAN统计信息
 */
int32 HAL_CAN_GetStats(hal_can_handle_t handle,
                       uint32 *tx_count,
                       uint32 *rx_count,
                       uint32 *err_count)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;

    if (impl == NULL)
        return OS_ERR_INVALID_ID;

    if (tx_count)  *tx_count = impl->tx_count;
    if (rx_count)  *rx_count = impl->rx_count;
    if (err_count) *err_count = impl->err_count;

    return OS_SUCCESS;
}
