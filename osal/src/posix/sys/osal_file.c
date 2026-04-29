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
    int result = open(pathname, flags, mode);
    return result;
}

int32_t OSAL_close(int32_t fd)
{
    int result = close(fd);
    return result;
}

osal_ssize_t OSAL_read(int32_t fd, void *buf, osal_size_t count)
{
    size_t read_count = count;
    ssize_t result = read(fd, buf, read_count);
    return result;
}

osal_ssize_t OSAL_write(int32_t fd, const void *buf, osal_size_t count)
{
    size_t write_count = count;
    ssize_t result = write(fd, buf, write_count);
    return result;
}

osal_ssize_t OSAL_lseek(int32_t fd, osal_ssize_t offset, int32_t whence)
{
    off_t seek_offset = offset;
    off_t result = lseek(fd, seek_offset, whence);
    return result;
}

/*===========================================================================
 * 文件控制操作
 *===========================================================================*/

int32_t OSAL_fcntl(int32_t fd, int32_t cmd, int32_t arg)
{
    int result = fcntl(fd, cmd, arg);
    return result;
}

/*===========================================================================
 * 设备控制操作
 *===========================================================================*/

int32_t OSAL_ioctl(int32_t fd, uint32_t request, void *argp)
{
    unsigned long ioctl_request = request;
    int result = ioctl(fd, ioctl_request, argp);
    return result;
}
