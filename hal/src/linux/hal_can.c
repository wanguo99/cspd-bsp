/************************************************************************
 * HAL层 - CAN驱动Linux实现
 ************************************************************************/

/* 必须在所有头文件之前定义，以启用完整的POSIX功能 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <net/if.h>           /* struct ifreq 定义 */
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/ioctl.h>        /* SIOCGIFINDEX 宏定义 */
#include "hal_can.h"
#include "osal.h"

/* 兼容性定义 */
#ifndef IFNAMSIZ
#define IFNAMSIZ IF_NAMESIZE
#endif

typedef struct
{
    int sockfd;
    uint32_t tx_count;
    uint32_t rx_count;
    uint32_t err_count;
    str_t interface[IFNAMSIZ];
    uint32_t baudrate;
    bool initialized;
} hal_can_context_t;

/**
 * @brief 初始化CAN驱动
 */
int32_t HAL_CAN_Init(const hal_can_config_t *config, hal_can_handle_t *handle)
{
    hal_can_context_t *impl;
    struct sockaddr_can addr;
    struct ifreq ifr;
    int ret;

    /* 参数检查 */
    if (config == NULL || handle == NULL)
        return OS_INVALID_POINTER;

    if (config->interface == NULL || OSAL_Strlen(config->interface) == 0)
        return OS_ERROR;

    if (OSAL_Strlen(config->interface) >= IFNAMSIZ)
        return OS_ERR_NAME_TOO_LONG;

    /* 分配句柄 */
    impl = (hal_can_context_t *)OSAL_Malloc(sizeof(hal_can_context_t));
    if (NULL == impl)
    {
        LOG_ERROR("HAL_CAN", "Failed to allocate memory");
        return OS_ERROR;
    }

    OSAL_Memset(impl, 0, sizeof(hal_can_context_t));
    OSAL_Strncpy(impl->interface, config->interface, IFNAMSIZ - 1);
    impl->interface[IFNAMSIZ - 1] = '\0';
    impl->baudrate = config->baudrate;
    impl->initialized = false;

    /* 创建SocketCAN */
    impl->sockfd = OSAL_socket(OSAL_PF_CAN, OSAL_SOCK_RAW, OSAL_CAN_RAW);
    if (impl->sockfd < 0)
    {
        LOG_ERROR("HAL_CAN", "Failed to create socket: %s", OSAL_StrError(OSAL_GetErrno()));
        OSAL_Free(impl);
        return OS_ERROR;
    }

    /* 获取接口索引 */
    OSAL_Memset(&ifr, 0, sizeof(ifr));
    OSAL_Strncpy(ifr.ifr_name, config->interface, IFNAMSIZ - 1);
    ret = OSAL_ioctl(impl->sockfd, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        LOG_ERROR("HAL_CAN", "Failed to get interface index: %s (interface: %s)",
                   OSAL_StrError(OSAL_GetErrno()), config->interface);
        /* 提示: CAN接口必须先启动 (sudo ip link set can0 up) */
        OSAL_close(impl->sockfd);
        OSAL_Free(impl);
        return OS_ERROR;
    }

    /* 绑定到CAN接口 */
    OSAL_Memset(&addr, 0, sizeof(addr));
    addr.can_family = OSAL_AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    ret = OSAL_bind(impl->sockfd, (const osal_sockaddr_t *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("HAL_CAN", "Failed to bind interface: %s", OSAL_StrError(OSAL_GetErrno()));
        OSAL_close(impl->sockfd);
        OSAL_Free(impl);
        return OS_ERROR;
    }

    /* 设置接收超时 */
    if (config->rx_timeout > 0)
    {
        struct timeval tv;
        tv.tv_sec = config->rx_timeout / 1000;
        tv.tv_usec = (config->rx_timeout % 1000) * 1000;

        if (OSAL_setsockopt(impl->sockfd, OSAL_SOL_SOCKET, OSAL_SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            LOG_WARN("HAL_CAN", "Failed to set receive timeout: %s", OSAL_StrError(OSAL_GetErrno()));
        }
    }

    if (config->tx_timeout > 0)
    {
        struct timeval tv;
        tv.tv_sec = config->tx_timeout / 1000;
        tv.tv_usec = (config->tx_timeout % 1000) * 1000;

        if (OSAL_setsockopt(impl->sockfd, OSAL_SOL_SOCKET, OSAL_SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        {
            LOG_WARN("HAL_CAN", "Failed to set send timeout: %s", OSAL_StrError(OSAL_GetErrno()));
        }
    }

    impl->initialized = true;
    *handle = (hal_can_handle_t)impl;

    LOG_INFO("HAL_CAN", "Initialized successfully: %s @ %u bps",
              config->interface, config->baudrate);
    return OS_SUCCESS;
}

/**
 * @brief 关闭CAN驱动
 */
int32_t HAL_CAN_Deinit(hal_can_handle_t handle)
{
    hal_can_context_t *impl = (hal_can_context_t *)handle;

    if (NULL == impl)
        return OS_ERR_INVALID_ID;

    if (impl->initialized && impl->sockfd >= 0)
    {
        OSAL_close(impl->sockfd);
        impl->sockfd = -1;
    }

    impl->initialized = false;
    OSAL_Free(impl);

    LOG_INFO("HAL_CAN", "Deinitialized successfully");
    return OS_SUCCESS;
}

/**
 * @brief 发送CAN帧
 */
int32_t HAL_CAN_Send(hal_can_handle_t handle, const can_frame_t *frame)
{
    hal_can_context_t *impl = (hal_can_context_t *)handle;
    struct can_frame can_frame;
    osal_ssize_t ret;

    /* 参数检查 */
    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    if (frame->dlc > 8)
    {
        LOG_ERROR("HAL_CAN", "Invalid DLC: %u", frame->dlc);
        return OS_ERROR;
    }

    /* 转换为SocketCAN格式 */
    OSAL_Memset(&can_frame, 0, sizeof(can_frame));
    can_frame.can_id = frame->can_id;
    can_frame.can_dlc = frame->dlc;
    OSAL_Memcpy(can_frame.data, frame->data, frame->dlc);

    /* 发送 */
    ret = OSAL_write(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret != sizeof(struct can_frame))
    {
        if (ret < 0)
        {
            LOG_ERROR("HAL_CAN", "Send failed: %s", OSAL_StrError(OSAL_GetErrno()));
        }
        else
        {
            LOG_ERROR("HAL_CAN", "Incomplete send: %d/%u bytes",
                       (int32_t)ret, (uint32_t)sizeof(struct can_frame));
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
int32_t HAL_CAN_Recv(hal_can_handle_t handle, can_frame_t *frame, int32_t timeout)
{
    hal_can_context_t *impl = (hal_can_context_t *)handle;
    struct can_frame can_frame;
    osal_ssize_t ret;

    /* 参数检查 */
    if (impl == NULL || frame == NULL)
        return OS_INVALID_POINTER;

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    /* 临时设置超时 */
    if (timeout >= 0)
    {
        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        if (OSAL_setsockopt(impl->sockfd, OSAL_SOL_SOCKET, OSAL_SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
            LOG_WARN("HAL_CAN", "Failed to set temporary receive timeout: %s", OSAL_StrError(OSAL_GetErrno()));
        }
    }

    /* 接收 */
    ret = OSAL_read(impl->sockfd, &can_frame, sizeof(struct can_frame));
    if (ret < 0)
    {
        int32_t err = OSAL_GetErrno();
        if (err == OSAL_EAGAIN || err == OSAL_EWOULDBLOCK)
            return OS_ERROR_TIMEOUT;

        LOG_ERROR("HAL_CAN", "Receive failed: %s", OSAL_StrError(err));
        impl->err_count++;
        return OS_ERROR;
    }

    if (ret != sizeof(struct can_frame))
    {
        LOG_ERROR("HAL_CAN", "Incomplete receive: %d/%u bytes",
                   (int32_t)ret, (uint32_t)sizeof(struct can_frame));
        impl->err_count++;
        return OS_ERROR;
    }

    /* 转换为内部格式 */
    OSAL_Memset(frame, 0, sizeof(can_frame_t));
    frame->can_id = can_frame.can_id;
    frame->dlc = can_frame.can_dlc;

    /* 防止越界 */
    if (can_frame.can_dlc > 8)
        can_frame.can_dlc = 8;

    OSAL_Memcpy(frame->data, can_frame.data, can_frame.can_dlc);
    frame->timestamp = OSAL_GetTickCount();

    impl->rx_count++;
    return OS_SUCCESS;
}

/**
 * @brief 设置CAN过滤器
 */
int32_t HAL_CAN_SetFilter(hal_can_handle_t handle, uint32_t filter_id, uint32_t filter_mask)
{
    hal_can_context_t *impl = (hal_can_context_t *)handle;
    struct can_filter rfilter[1];

    if (NULL == impl)
        return OS_ERR_INVALID_ID;

    if (!impl->initialized || impl->sockfd < 0)
        return OS_ERR_INVALID_ID;

    rfilter[0].can_id = filter_id;
    rfilter[0].can_mask = filter_mask;

    if (OSAL_setsockopt(impl->sockfd, OSAL_SOL_CAN_RAW, OSAL_CAN_RAW_FILTER,
                   &rfilter, sizeof(rfilter)) < 0)
    {
        LOG_ERROR("HAL_CAN", "Failed to set filter: %s", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    LOG_INFO("HAL_CAN", "Filter set: ID=0x%X, Mask=0x%X",
              filter_id, filter_mask);
    return OS_SUCCESS;
}

/**
 * @brief 获取CAN统计信息
 */
int32_t HAL_CAN_GetStats(hal_can_handle_t handle,
                       uint32_t *tx_count,
                       uint32_t *rx_count,
                       uint32_t *err_count)
{
    hal_can_context_t *impl = (hal_can_context_t *)handle;

    if (NULL == impl)
        return OS_ERR_INVALID_ID;

    if (!impl->initialized)
        return OS_ERR_INVALID_ID;

    if (tx_count)  *tx_count = impl->tx_count;
    if (rx_count)  *rx_count = impl->rx_count;
    if (err_count) *err_count = impl->err_count;

    return OS_SUCCESS;
}
