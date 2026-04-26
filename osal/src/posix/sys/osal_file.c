/************************************************************************
 * OSAL - 文件I/O操作封装实现（POSIX）
 ************************************************************************/

#include "sys/osal_file.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*===========================================================================
 * 文件I/O操作
 *===========================================================================*/

int32_t OSAL_open(const char *pathname, int32_t flags, uint32_t mode)
{
    return (int32_t)open(pathname, flags, mode);
}

int32_t OSAL_close(int32_t fd)
{
    return (int32_t)close(fd);
}

osal_ssize_t OSAL_read(int32_t fd, void *buf, osal_size_t count)
{
    return (osal_ssize_t)read(fd, buf, (size_t)count);
}

osal_ssize_t OSAL_write(int32_t fd, const void *buf, osal_size_t count)
{
    return (osal_ssize_t)write(fd, buf, (size_t)count);
}

osal_ssize_t OSAL_lseek(int32_t fd, osal_ssize_t offset, int32_t whence)
{
    return (osal_ssize_t)lseek(fd, (off_t)offset, whence);
}

/*===========================================================================
 * 文件控制操作
 *===========================================================================*/

int32_t OSAL_fcntl(int32_t fd, int32_t cmd, int32_t arg)
{
    return (int32_t)fcntl(fd, cmd, arg);
}

/*===========================================================================
 * 设备控制操作
 *===========================================================================*/

int32_t OSAL_ioctl(int32_t fd, uint32_t request, void *argp)
{
    return (int32_t)ioctl(fd, (unsigned long)request, argp);
}
