/************************************************************************
 * OSAL - unistd系统调用封装实现（Linux）
 ************************************************************************/

#define _DEFAULT_SOURCE  /* 启用usleep等函数 */
#include "sys/osal_unistd.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/*===========================================================================
 * 文件I/O操作
 *===========================================================================*/

int32 OSAL_open(const char *pathname, int32 flags, uint32 mode)
{
    return (int32)open(pathname, flags, mode);
}

int32 OSAL_close(int32 fd)
{
    return (int32)close(fd);
}

osal_ssize_t OSAL_read(int32 fd, void *buf, osal_size_t count)
{
    return (osal_ssize_t)read(fd, buf, (size_t)count);
}

osal_ssize_t OSAL_write(int32 fd, const void *buf, osal_size_t count)
{
    return (osal_ssize_t)write(fd, buf, (size_t)count);
}

osal_ssize_t OSAL_lseek(int32 fd, osal_ssize_t offset, int32 whence)
{
    return (osal_ssize_t)lseek(fd, (off_t)offset, whence);
}

int32 OSAL_usleep(uint32 usec)
{
    return usleep(usec);
}

/*===========================================================================
 * 文件控制操作
 *===========================================================================*/

int32 OSAL_fcntl(int32 fd, int32 cmd, int32 arg)
{
    return (int32)fcntl(fd, cmd, arg);
}

/*===========================================================================
 * 设备控制操作
 *===========================================================================*/

int32 OSAL_ioctl(int32 fd, uint32 request, void *argp)
{
    return (int32)ioctl(fd, (unsigned long)request, argp);
}
