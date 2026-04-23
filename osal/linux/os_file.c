/************************************************************************
 * 文件I/O实现 (Linux)
 ************************************************************************/

#include "osal.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#define MAX_FILE_DESCRIPTORS  64

typedef struct
{
    bool   in_use;
    int32  native_fd;
    char   path[256];
} file_record_t;

static file_record_t g_file_table[MAX_FILE_DESCRIPTORS];
static osal_id_t     g_file_mutex = OS_OBJECT_ID_UNDEFINED;

static int32 convert_mode_to_flags(uint32 mode, uint32 flags)
{
    int32 native_flags = 0;

    switch (mode)
    {
        case OS_FILE_MODE_READ:
            native_flags = O_RDONLY;
            break;
        case OS_FILE_MODE_WRITE:
            native_flags = O_WRONLY;
            break;
        case OS_FILE_MODE_RDWR:
            native_flags = O_RDWR;
            break;
        default:
            return -1;
    }

    if (flags & OS_FILE_FLAG_CREATE)
        native_flags |= O_CREAT;
    if (flags & OS_FILE_FLAG_TRUNCATE)
        native_flags |= O_TRUNC;
    if (flags & OS_FILE_FLAG_APPEND)
        native_flags |= O_APPEND;
    if (flags & OS_FILE_FLAG_NONBLOCK)
        native_flags |= O_NONBLOCK;

    return native_flags;
}

int32 OS_FileOpen(osal_id_t *fd, const char *path, uint32 mode, uint32 flags)
{
    int32 native_flags;
    int32 native_fd;
    uint32 i;

    if (fd == NULL || path == NULL)
        return OS_INVALID_POINTER;

    native_flags = convert_mode_to_flags(mode, flags);
    if (native_flags < 0)
        return OS_ERROR;

    native_fd = open(path, native_flags, 0666);
    if (native_fd < 0)
    {
        OS_printf("[OSAL] 打开文件失败: %s, errno=%d\n", path, errno);
        return OS_ERROR;
    }

    if (g_file_mutex == OS_OBJECT_ID_UNDEFINED)
    {
        OS_MutexCreate(&g_file_mutex, "FILE_MTX", 0);
    }

    OS_MutexLock(g_file_mutex);

    for (i = 0; i < MAX_FILE_DESCRIPTORS; i++)
    {
        if (!g_file_table[i].in_use)
        {
            g_file_table[i].in_use = true;
            g_file_table[i].native_fd = native_fd;
            strncpy(g_file_table[i].path, path, sizeof(g_file_table[i].path) - 1);
            g_file_table[i].path[sizeof(g_file_table[i].path) - 1] = '\0';
            *fd = i + 1;
            OS_MutexUnlock(g_file_mutex);
            return OS_SUCCESS;
        }
    }

    OS_MutexUnlock(g_file_mutex);
    close(native_fd);
    return OS_ERR_NO_FREE_IDS;
}

int32 OS_FileClose(osal_id_t fd)
{
    uint32 index;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    close(g_file_table[index].native_fd);
    g_file_table[index].in_use = false;
    g_file_table[index].native_fd = -1;

    OS_MutexUnlock(g_file_mutex);
    return OS_SUCCESS;
}

int32 OS_FileRead(osal_id_t fd, void *buffer, uint32 size)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_read;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL)
        return OS_INVALID_POINTER;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    bytes_read = read(native_fd, buffer, (size_t)size);
    if (bytes_read < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    return bytes_read;
}

int32 OS_FileWrite(osal_id_t fd, const void *buffer, uint32 size)
{
    uint32 index;
    int32 native_fd;
    int32 bytes_written;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    if (buffer == NULL)
        return OS_INVALID_POINTER;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    bytes_written = write(native_fd, buffer, (size_t)size);
    if (bytes_written < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OS_ERR_WOULD_BLOCK;
        return OS_ERROR;
    }

    return bytes_written;
}

int32 OS_FileSeek(osal_id_t fd, int32 offset, uint32 whence)
{
    uint32 index;
    int32 native_fd;
    int32 native_whence;
    int32 result;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    index = fd - 1;

    switch (whence)
    {
        case OS_FILE_SEEK_SET:
            native_whence = SEEK_SET;
            break;
        case OS_FILE_SEEK_CUR:
            native_whence = SEEK_CUR;
            break;
        case OS_FILE_SEEK_END:
            native_whence = SEEK_END;
            break;
        default:
            return OS_ERROR;
    }

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    result = lseek(native_fd, offset, native_whence);
    if (result < 0)
        return OS_ERROR;

    return result;
}

int32 OS_FileSetFlags(osal_id_t fd, uint32 flags)
{
    uint32 index;
    int32 native_fd;
    int32 current_flags;
    int32 new_flags;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    current_flags = fcntl(native_fd, F_GETFL, 0);
    if (current_flags < 0)
        return OS_ERROR;

    new_flags = current_flags;

    if (flags & OS_FILE_FLAG_NONBLOCK)
        new_flags |= O_NONBLOCK;
    else
        new_flags &= ~O_NONBLOCK;

    if (flags & OS_FILE_FLAG_APPEND)
        new_flags |= O_APPEND;
    else
        new_flags &= ~O_APPEND;

    if (fcntl(native_fd, F_SETFL, new_flags) < 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32 OS_FileGetFlags(osal_id_t fd, uint32 *flags)
{
    uint32 index;
    int32 native_fd;
    int32 native_flags;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    if (flags == NULL)
        return OS_INVALID_POINTER;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    native_flags = fcntl(native_fd, F_GETFL, 0);
    if (native_flags < 0)
        return OS_ERROR;

    *flags = 0;
    if (native_flags & O_NONBLOCK)
        *flags |= OS_FILE_FLAG_NONBLOCK;
    if (native_flags & O_APPEND)
        *flags |= OS_FILE_FLAG_APPEND;

    return OS_SUCCESS;
}

int32 OS_FileIoctl(osal_id_t fd, os_ioctl_req_t request, void *arg)
{
    uint32 index;
    int32 native_fd;
    int32 result;

    if (fd == 0 || fd > MAX_FILE_DESCRIPTORS)
        return OS_ERR_INVALID_ID;

    index = fd - 1;

    OS_MutexLock(g_file_mutex);

    if (!g_file_table[index].in_use)
    {
        OS_MutexUnlock(g_file_mutex);
        return OS_ERR_INVALID_ID;
    }

    native_fd = g_file_table[index].native_fd;
    OS_MutexUnlock(g_file_mutex);

    result = ioctl(native_fd, (unsigned long)request, arg);
    if (result < 0)
        return OS_ERROR;

    return OS_SUCCESS;
}

int32 OS_FileSelect(const osal_id_t *fds, uint32 nfds,
                    uint32 *read_set, uint32 *write_set, uint32 *error_set,
                    int32 timeout_ms)
{
    fd_set readfds, writefds, errorfds;
    struct timeval tv;
    struct timeval *tv_ptr;
    int32 max_fd = -1;
    int32 result;
    uint32 i;

    if (fds == NULL || nfds == 0 || nfds > 32)
        return OS_ERROR;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);

    OS_MutexLock(g_file_mutex);

    for (i = 0; i < nfds; i++)
    {
        uint32 index;
        int32 native_fd;

        if (fds[i] == 0 || fds[i] > MAX_FILE_DESCRIPTORS)
            continue;

        index = fds[i] - 1;

        if (!g_file_table[index].in_use)
            continue;

        native_fd = g_file_table[index].native_fd;

        if (read_set && (*read_set & (1U << i)))
            FD_SET(native_fd, &readfds);

        if (write_set && (*write_set & (1U << i)))
            FD_SET(native_fd, &writefds);

        if (error_set && (*error_set & (1U << i)))
            FD_SET(native_fd, &errorfds);

        if (native_fd > max_fd)
            max_fd = native_fd;
    }

    OS_MutexUnlock(g_file_mutex);

    if (max_fd < 0)
        return OS_ERROR;

    if (timeout_ms == OS_PEND)
    {
        tv_ptr = NULL;
    }
    else
    {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tv_ptr = &tv;
    }

    result = select(max_fd + 1, &readfds, &writefds, &errorfds, tv_ptr);

    if (result < 0)
    {
        if (errno == EINTR)
            return OS_ERR_INTERRUPTED;
        return OS_ERROR;
    }

    if (result == 0)
        return OS_ERROR_TIMEOUT;

    if (read_set)
        *read_set = 0;
    if (write_set)
        *write_set = 0;
    if (error_set)
        *error_set = 0;

    OS_MutexLock(g_file_mutex);

    for (i = 0; i < nfds; i++)
    {
        uint32 index;
        int32 native_fd;

        if (fds[i] == 0 || fds[i] > MAX_FILE_DESCRIPTORS)
            continue;

        index = fds[i] - 1;

        if (!g_file_table[index].in_use)
            continue;

        native_fd = g_file_table[index].native_fd;

        if (read_set && FD_ISSET(native_fd, &readfds))
            *read_set |= (1U << i);

        if (write_set && FD_ISSET(native_fd, &writefds))
            *write_set |= (1U << i);

        if (error_set && FD_ISSET(native_fd, &errorfds))
            *error_set |= (1U << i);
    }

    OS_MutexUnlock(g_file_mutex);

    return result;
}
