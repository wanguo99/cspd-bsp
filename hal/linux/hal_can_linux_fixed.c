/************************************************************************
 * HAL层 - CAN驱动Linux实现（修复版）
 *
 * 修复内容：
 * 1. 启用CAN初始化代码
 * 2. 添加完善的错误处理
 * 3. 添加参数验证
 * 4. 改进资源管理
 ************************************************************************/

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

/*
 * CAN句柄结构
 */
typedef struct
{
    int sockfd;
    uint32 tx_count;
    uint32 rx_count;
    uint32 err_count;
    char interface[IFNAMSIZ];
    uint32 baudrate;
    bool initialized;
} can_handle_impl_t;

/**
 * @brief 初始化CAN驱动
 */
int32 HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle)
{
    can_handle_impl_t *impl;
    struct sockaddr_can addr;
    struct ifreq ifr;
    int ret;

    /* 参数检查 */
    if (config == NULL || handle == NULL)
        return OS_INVALID_POINTER;

    if (config->interface == NULL || strlen(config->interface) == 0)
        return OS_ERROR;

    if (strlen(config->interface) >= IFNAMSIZ)
        return OS_ERR_NAME_TOO_LONG;

    /* 分配句柄 */
    impl = malloc(sizeof(can_handle_impl_t));
    if (impl == NULL)
    {
        OS_printf("[HAL CAN] 内存分配失败\n");
        return OS_ERROR;
    }

    memset(impl, 0, sizeof(can_handle_impl_t));
    strncpy(impl->interface, config->interface, IFNAMSIZ - 1);
    impl->interface[IFNAMSIZ - 1] = '\0';
    impl->baudrate = config->baudrate;
    impl->initialized = false;

    /* 创建SocketCAN */
    impl->sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (impl->sockfd < 0)
    {
        OS_printf("[HAL CAN] 创建socket失败: %s\n", strerror(errno));
        free(impl);
        return OS_ERROR;
    }

    /* 获取接口索引 */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, config->interface, IFNAMSIZ - 1);
    ret = ioctl(impl->sockfd, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        OS_printf("[HAL CAN] 获取接口索引失败: %s (接口: %s)\n",
                 strerror(errno), config->interface);
        OS_printf("[HAL CAN] 提示: 请确保CAN接口已启动 (sudo ip link set %s up)\n",
                 config->interface);
        close(impl->sockfd);
        free(impl);
        return OS_ERROR;
    }

    /* 绑定到CAN接口 */
    memset(&addr, 0, sizeof(addr));
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
    if (config->rx_timeout > 0)
    {
        struct timeval tv;
        tv.tv_sec = config->rx_timeout / 1000;
        tv.tv_usec = (config->rx_timeout % 1000) * 1000;

        if (setsockopt(impl->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            OS_printf("[HAL CAN] 设置接收超时失败: %s\n", strerror(errno));
            /* 非致命错误，继续 */
        }
    }

    /* 设置发送超时 */
    if (config->tx_timeout > 0)
    {
        struct timeval tv;
        tv.tv_sec = config->tx_timeout / 1000;
        tv.tv_usec = (config->tx_timeout % 1000) * 1000;

        if (setsockopt(impl->sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        {
            OS_printf("[HAL CAN] 设置发送超时失败: %s\n", strerror(errno));
            /* 非致命错误，继续 */
        }
    }

    impl->initialized = true;
    *handle = (hal_can_handle_t)impl;

    OS_printf("[HAL CAN] 初始化成功: %s @ %u bps\n",
             config->interface, config->baudrate);
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

    if (impl->initialized && impl->sockfd >= 0)
    {
        close(impl->sockfd);
        impl->sockfd = -1;
    }

    impl->initialized = false;
    free(impl);

    OS_printf("[HAL CAN] 已关闭\n");
    return OS_SUCCESS;
}

/**
 * @brief 发送CAN帧
 */
int32 HAL_CAN_Send(hal_can_handle_t handle, const can_frame_t *frame)
{
    can_handle_impl_t *impl = (can_handle_impl_t *)handle;
    struct can_frame can_frame;
    ssize_t ret;

    /* 参数检查 */
    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    if (frame->dlc > 8)
    {
        OS_printf("[HAL CAN] 无效的DLC: %u\n", frame->dlc);
        return OS_ERROR;
    }

    /* 转换为SocketCAN格式 */
    memset(&can_frame, 0, sizeof(can_frame));
    can_frame.can_id = frame->can_id;
    can_frame.can_dlc = frame->dlc;
    memcpy(can_frame.data, &frame->msg, frame->dlc);

    /* 发送 */
    ret = write(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret != sizeof(struct can_frame))
    {
        if (ret < 0)
        {
            OS_printf("[HAL CAN] 发送失败: %s\n", strerror(errno));
        }
        else
        {
            OS_printf("[HAL CAN] 发送不完整: %zd/%zu 字节\n",
                     ret, sizeof(struct can_frame));
        }
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
    ssize_t ret;

    /* 参数检查 */
    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    /* 如果指定了超时，临时设置 */
    if (timeout >= 0)
    {
        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        setsockopt(impl->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    /* 接收 */
    ret = read(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERROR_TIMEOUT;

        OS_printf("[HAL CAN] 接收失败: %s\n", strerror(errno));
        impl->err_count++;
        return OS_ERROR;
    }

    if (ret != sizeof(struct can_frame))
    {
        OS_printf("[HAL CAN] 接收不完整: %zd/%zu 字节\n",
                 ret, sizeof(struct can_frame));
        impl->err_count++;
        return OS_ERROR;
    }

    /* 转换为内部格式 */
    memset(frame, 0, sizeof(can_frame_t));
    frame->can_id = can_frame.can_id;
    frame->dlc = can_frame.can_dlc;

    /* 确保不会越界 */
    if (can_frame.can_dlc > 8)
        can_frame.can_dlc = 8;

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

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    rfilter[0].can_id = filter_id;
    rfilter[0].can_mask = filter_mask;

    if (setsockopt(impl->sockfd, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &rfilter, sizeof(rfilter)) < 0)
    {
        OS_printf("[HAL CAN] 设置过滤器失败: %s\n", strerror(errno));
        return OS_ERROR;
    }

    OS_printf("[HAL CAN] 过滤器已设置: ID=0x%X, Mask=0x%X\n",
             filter_id, filter_mask);
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

    if (!impl->initialized)
        return OS_ERR_INVALID_ID;

    if (tx_count)  *tx_count = impl->tx_count;
    if (rx_count)  *rx_count = impl->rx_count;
    if (err_count) *err_count = impl->err_count;

    return OS_SUCCESS;
}
