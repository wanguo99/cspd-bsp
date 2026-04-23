/************************************************************************
 * HAL层 - Linux串口驱动实现
 *
 * 基于POSIX termios实现
 ************************************************************************/

#include "hal_serial.h"
#include "osal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>

typedef struct
{
    int fd;
    hal_serial_config_t config;
    char device[64];
} hal_serial_context_t;

static speed_t get_baudrate(uint32 baudrate)
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
    struct termios tty;
    speed_t speed;

    if (device == NULL || handle == NULL)
    {
        return OS_INVALID_POINTER;
    }

    if (config == NULL)
    {
        return OS_INVALID_POINTER;
    }

    ctx = (hal_serial_context_t *)malloc(sizeof(hal_serial_context_t));
    if (ctx == NULL)
    {
        OS_printf("[HAL_Serial] Failed to allocate context\n");
        return OS_ERR_NO_MEMORY;
    }

    memset(ctx, 0, sizeof(hal_serial_context_t));

    /* 打开串口设备 */
    ctx->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (ctx->fd < 0)
    {
        OS_printf("[HAL_Serial] Failed to open %s: %s\n", device, strerror(errno));
        free(ctx);
        return OS_ERROR;
    }

    /* 设置为阻塞模式 */
    fcntl(ctx->fd, F_SETFL, 0);

    /* 获取当前配置 */
    if (tcgetattr(ctx->fd, &tty) != 0)
    {
        OS_printf("[HAL_Serial] Failed to get attributes: %s\n", strerror(errno));
        close(ctx->fd);
        free(ctx);
        return OS_ERROR;
    }

    /* 配置波特率 */
    speed = get_baudrate(config->baud_rate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    /* 配置数据位 */
    tty.c_cflag &= ~CSIZE;
    if (config->data_bits == 7)
    {
        tty.c_cflag |= CS7;
    }
    else
    {
        tty.c_cflag |= CS8;  /* 默认8位 */
    }

    /* 配置停止位 */
    if (config->stop_bits == 2)
    {
        tty.c_cflag |= CSTOPB;
    }
    else
    {
        tty.c_cflag &= ~CSTOPB;  /* 默认1位 */
    }

    /* 配置校验位 */
    if (config->parity == 1)  /* 奇校验 */
    {
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
    }
    else if (config->parity == 2)  /* 偶校验 */
    {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    }
    else  /* 无校验 */
    {
        tty.c_cflag &= ~PARENB;
    }

    /* 启用接收，本地模式 */
    tty.c_cflag |= (CLOCAL | CREAD);

    /* 禁用硬件流控 */
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    /* 原始模式 */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;

    /* 超时配置 */
    tty.c_cc[VMIN]  = 0;  /* 非阻塞读取 */
    tty.c_cc[VTIME] = 0;  /* 不等待 */

    /* 应用配置 */
    if (tcsetattr(ctx->fd, TCSANOW, &tty) != 0)
    {
        OS_printf("[HAL_Serial] Failed to set attributes: %s\n", strerror(errno));
        close(ctx->fd);
        free(ctx);
        return OS_ERROR;
    }

    /* 清空缓冲区 */
    tcflush(ctx->fd, TCIOFLUSH);

    /* 保存配置 */
    memcpy(&ctx->config, config, sizeof(hal_serial_config_t));
    strncpy(ctx->device, device, sizeof(ctx->device) - 1);
    ctx->device[sizeof(ctx->device) - 1] = '\0';

    *handle = (hal_serial_handle_t)ctx;

    OS_printf("[HAL_Serial] Opened %s (baudrate=%u, databits=%u, stopbits=%u, parity=%u)\n",
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
        close(ctx->fd);
        OS_printf("[HAL_Serial] Closed %s\n", ctx->device);
    }

    free(ctx);
    return OS_SUCCESS;
}

/************************************************************************
 * HAL_Serial_Write - 发送数据
 ************************************************************************/
int32 HAL_Serial_Write(hal_serial_handle_t handle, const void *buffer, uint32 size, int32 timeout)
{
    hal_serial_context_t *ctx = (hal_serial_context_t *)handle;
    fd_set writefds;
    struct timeval tv;
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
        FD_ZERO(&writefds);
        FD_SET(ctx->fd, &writefds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ret = select(ctx->fd + 1, NULL, &writefds, NULL, &tv);
        if (ret == 0)
        {
            return OS_ERROR_TIMEOUT;
        }
        else if (ret < 0)
        {
            OS_printf("[HAL_Serial] Select error: %s\n", strerror(errno));
            return OS_ERROR;
        }
    }

    /* 发送数据 */
    written = write(ctx->fd, buffer, size);
    if (written < 0)
    {
        OS_printf("[HAL_Serial] Write error: %s\n", strerror(errno));
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
    fd_set readfds;
    struct timeval tv;
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
        FD_ZERO(&readfds);
        FD_SET(ctx->fd, &readfds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ret = select(ctx->fd + 1, &readfds, NULL, NULL, &tv);
        if (ret == 0)
        {
            return OS_ERROR_TIMEOUT;
        }
        else if (ret < 0)
        {
            OS_printf("[HAL_Serial] Select error: %s\n", strerror(errno));
            return OS_ERROR;
        }
    }

    /* 接收数据 */
    nread = read(ctx->fd, buffer, size);
    if (nread < 0)
    {
        OS_printf("[HAL_Serial] Read error: %s\n", strerror(errno));
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
    if (tcflush(ctx->fd, TCIOFLUSH) != 0)
    {
        OS_printf("[HAL_Serial] Flush error: %s\n", strerror(errno));
        return OS_ERROR;
    }

    return OS_SUCCESS;
}
