/************************************************************************
 * HAL层 - Linux串口驱动实现
 *
 * 基于POSIX termios实现
 ************************************************************************/

#include "hal_serial.h"
#include "osal.h"
#include <termios.h>  /* 系统波特率常量 B9600 等 */

typedef struct
{
    int fd;
    hal_serial_config_t config;
    char device[64];
} hal_serial_context_t;

static uint32 hal_serial_get_baudrate(uint32 baudrate)
{
    switch (baudrate)
    {
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 57600:   return B57600;
        case 115200:  return B115200;
        case 230400:  return B230400;
        case 460800:  return B460800;
        case 500000:  return B500000;
        case 576000:  return B576000;
        case 921600:  return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default:      return B115200;
    }
}

int32 HAL_Serial_Open(const char *device, const hal_serial_config_t *config, hal_serial_handle_t *handle)
{
    hal_serial_context_t *ctx;
    osal_termios_t tty;
    uint32 speed;

    if (device == NULL || handle == NULL)
    {
        return OS_INVALID_POINTER;
    }

    if (config == NULL)
    {
        return OS_INVALID_POINTER;
    }

    ctx = (hal_serial_context_t *)OSAL_Malloc(sizeof(hal_serial_context_t));
    if (ctx == NULL)
    {
        OSAL_LogError("HAL_Serial", "Failed to allocate context");
        return OS_ERR_NO_MEMORY;
    }

    OSAL_Memset(ctx, 0, sizeof(hal_serial_context_t));

    /* 打开串口设备 */
    ctx->fd = OSAL_open(device, OSAL_O_RDWR | OSAL_O_NOCTTY | OSAL_O_NONBLOCK, 0);
    if (ctx->fd < 0)
    {
        OSAL_LogError("HAL_Serial", "Failed to open %s: %s", device, OSAL_StrError(OSAL_GetErrno()));
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 设置为阻塞模式 */
    OSAL_fcntl(ctx->fd, OSAL_F_SETFL, 0);

    /* 获取当前配置 */
    if (OSAL_tcgetattr(ctx->fd, &tty) != 0)
    {
        OSAL_LogError("HAL_Serial", "Failed to get attributes: %s", OSAL_StrError(OSAL_GetErrno()));
        OSAL_close(ctx->fd);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 配置波特率 */
    speed = hal_serial_get_baudrate(config->baud_rate);
    OSAL_cfsetispeed(&tty, speed);
    OSAL_cfsetospeed(&tty, speed);

    /* 配置数据位 */
    tty.c_cflag &= ~OSAL_CSIZE;
    if (config->data_bits == 7)
    {
        tty.c_cflag |= OSAL_CS7;
    }
    else
    {
        tty.c_cflag |= OSAL_CS8;  /* 默认8位 */
    }

    /* 配置停止位 */
    if (config->stop_bits == 2)
    {
        tty.c_cflag |= OSAL_CSTOPB;
    }
    else
    {
        tty.c_cflag &= ~OSAL_CSTOPB;  /* 默认1位 */
    }

    /* 配置校验位 */
    if (config->parity == 1)  /* 奇校验 */
    {
        tty.c_cflag |= OSAL_PARENB;
        tty.c_cflag |= OSAL_PARODD;
    }
    else if (config->parity == 2)  /* 偶校验 */
    {
        tty.c_cflag |= OSAL_PARENB;
        tty.c_cflag &= ~OSAL_PARODD;
    }
    else  /* 无校验 */
    {
        tty.c_cflag &= ~OSAL_PARENB;
    }

    /* 启用接收，本地模式 */
    tty.c_cflag |= (OSAL_CLOCAL | OSAL_CREAD);

    /* 禁用硬件流控 */
#ifdef OSAL_CRTSCTS
    tty.c_cflag &= ~OSAL_CRTSCTS;
#endif

    /* 原始模式 */
    tty.c_lflag &= ~(OSAL_ICANON | OSAL_ECHO | OSAL_ECHOE | OSAL_ISIG);
    tty.c_iflag &= ~(OSAL_IXON | OSAL_IXOFF | OSAL_IXANY);
    tty.c_iflag &= ~(OSAL_IGNBRK | OSAL_BRKINT | OSAL_PARMRK | OSAL_ISTRIP | OSAL_INLCR | OSAL_IGNCR | OSAL_ICRNL);
    tty.c_oflag &= ~OSAL_OPOST;

    /* 超时配置 */
    tty.c_cc[OSAL_VMIN]  = 0;  /* 非阻塞读取 */
    tty.c_cc[OSAL_VTIME] = 0;  /* 不等待 */

    /* 应用配置 */
    if (OSAL_tcsetattr(ctx->fd, OSAL_TCSANOW, &tty) != 0)
    {
        OSAL_LogError("HAL_Serial", "Failed to set attributes: %s", OSAL_StrError(OSAL_GetErrno()));
        OSAL_close(ctx->fd);
        OSAL_Free(ctx);
        return OS_ERROR;
    }

    /* 清空缓冲区 */
    OSAL_tcflush(ctx->fd, OSAL_TCIOFLUSH);

    /* 保存配置 */
    OSAL_Memcpy(&ctx->config, config, sizeof(hal_serial_config_t));
    OSAL_Strncpy(ctx->device, device, sizeof(ctx->device) - 1);
    ctx->device[sizeof(ctx->device) - 1] = '\0';

    *handle = (hal_serial_handle_t)ctx;

    OSAL_LogInfo("HAL_Serial", "Opened %s (baudrate=%u, databits=%u, stopbits=%u, parity=%u)",
              device, config->baud_rate, config->data_bits, config->stop_bits, config->parity);

    return OS_SUCCESS;
}

/************************************************************************
 * HAL_Serial_Close - 关闭串口
 ************************************************************************/
int32 HAL_Serial_Close(hal_serial_handle_t handle)
{
    hal_serial_context_t *ctx = (hal_serial_context_t *)handle;

    if (ctx == NULL)
    {
        return OS_ERR_INVALID_ID;
    }

    if (ctx->fd >= 0)
    {
        OSAL_close(ctx->fd);
        OSAL_LogInfo("HAL_Serial", "Closed %s", ctx->device);
    }

    OSAL_Free(ctx);
    return OS_SUCCESS;
}

/************************************************************************
 * HAL_Serial_Write - 发送数据
 ************************************************************************/
int32 HAL_Serial_Write(hal_serial_handle_t handle, const void *buffer, uint32 size, int32 timeout)
{
    hal_serial_context_t *ctx = (hal_serial_context_t *)handle;
    osal_fd_set_t writefds;
    osal_timeval_t tv;
    int ret;
    int32 written;

    if (ctx == NULL || buffer == NULL)
    {
        return OS_INVALID_POINTER;
    }

    if (ctx->fd < 0)
    {
        return OS_ERR_INVALID_ID;
    }

    /* 设置超时 */
    if (timeout > 0)
    {
        OSAL_FD_ZERO(&writefds);
        OSAL_FD_SET(ctx->fd, &writefds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ret = OSAL_select(ctx->fd + 1, NULL, &writefds, NULL, &tv);
        if (ret == 0)
        {
            return OS_ERROR_TIMEOUT;
        }
        else if (ret < 0)
        {
            OSAL_LogError("HAL_Serial", "Select error: %s", OSAL_StrError(OSAL_GetErrno()));
            return OS_ERROR;
        }
    }

    /* 发送数据 */
    written = OSAL_write(ctx->fd, buffer, size);
    if (written < 0)
    {
        OSAL_LogError("HAL_Serial", "Write error: %s", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    return (int32)written;
}

/************************************************************************
 * HAL_Serial_Read - 接收数据
 ************************************************************************/
int32 HAL_Serial_Read(hal_serial_handle_t handle, void *buffer, uint32 size, int32 timeout)
{
    hal_serial_context_t *ctx = (hal_serial_context_t *)handle;
    osal_fd_set_t readfds;
    osal_timeval_t tv;
    int ret;
    int32 nread;

    if (ctx == NULL || buffer == NULL)
    {
        return OS_INVALID_POINTER;
    }

    if (ctx->fd < 0)
    {
        return OS_ERR_INVALID_ID;
    }

    /* 设置超时 */
    if (timeout > 0)
    {
        OSAL_FD_ZERO(&readfds);
        OSAL_FD_SET(ctx->fd, &readfds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ret = OSAL_select(ctx->fd + 1, &readfds, NULL, NULL, &tv);
        if (ret == 0)
        {
            return OS_ERROR_TIMEOUT;
        }
        else if (ret < 0)
        {
            OSAL_LogError("HAL_Serial", "Select error: %s", OSAL_StrError(OSAL_GetErrno()));
            return OS_ERROR;
        }
    }

    /* 接收数据 */
    nread = OSAL_read(ctx->fd, buffer, size);
    if (nread < 0)
    {
        OSAL_LogError("HAL_Serial", "Read error: %s", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    return (int32)nread;
}

/************************************************************************
 * HAL_Serial_Flush - 清空接收缓冲区
 ************************************************************************/
int32 HAL_Serial_Flush(hal_serial_handle_t handle)
{
    hal_serial_context_t *ctx = (hal_serial_context_t *)handle;

    if (ctx == NULL)
    {
        return OS_ERR_INVALID_ID;
    }

    if (ctx->fd < 0)
    {
        return OS_ERR_INVALID_ID;
    }

    /* 清空输入输出缓冲区 */
    if (OSAL_tcflush(ctx->fd, OSAL_TCIOFLUSH) != 0)
    {
        OSAL_LogError("HAL_Serial", "Flush error: %s", OSAL_StrError(OSAL_GetErrno()));
        return OS_ERROR;
    }

    return OS_SUCCESS;
}
