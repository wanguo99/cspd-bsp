/************************************************************************
 * OSAL - termios系统调用封装
 *
 * 功能：
 * - 封装终端控制函数
 * - 1:1映射系统调用，不引入业务逻辑
 * - 使用固定大小类型，避免平台相关类型
 *
 * 设计原则：
 * - 提供标准termios函数的封装
 * - 返回值与系统调用保持一致
 * - 便于RTOS移植
 ************************************************************************/

#ifndef OSAL_TERMIOS_H
#define OSAL_TERMIOS_H

#include "osal_types.h"

/*
 * 终端属性结构（平台无关）
 */
typedef struct {
    uint32_t c_iflag;      /* 输入模式标志 */
    uint32_t c_oflag;      /* 输出模式标志 */
    uint32_t c_cflag;      /* 控制模式标志 */
    uint32_t c_lflag;      /* 本地模式标志 */
    uint8_t  c_cc[32];     /* 控制字符 */
    uint32_t c_ispeed;     /* 输入波特率 */
    uint32_t c_ospeed;     /* 输出波特率 */
} osal_termios_t;

/*
 * 终端控制标志（与POSIX兼容）
 */
/* c_iflag 输入模式标志 */
#define OSAL_IGNBRK   0x00000001  /* 忽略BREAK */
#define OSAL_BRKINT   0x00000002  /* BREAK产生SIGINT */
#define OSAL_IGNPAR   0x00000004  /* 忽略奇偶校验错误 */
#define OSAL_PARMRK   0x00000008  /* 标记奇偶校验错误 */
#define OSAL_INPCK    0x00000010  /* 启用输入奇偶校验 */
#define OSAL_ISTRIP   0x00000020  /* 剥离第8位 */
#define OSAL_INLCR    0x00000040  /* 将NL映射为CR */
#define OSAL_IGNCR    0x00000080  /* 忽略CR */
#define OSAL_ICRNL    0x00000100  /* 将CR映射为NL */
#define OSAL_IXON     0x00000400  /* 启用输出软件流控 */
#define OSAL_IXANY    0x00000800  /* 任意字符重启输出 */
#define OSAL_IXOFF    0x00001000  /* 启用输入软件流控 */

/* c_oflag 输出模式标志 */
#define OSAL_OPOST    0x00000001  /* 执行输出处理 */

/* c_cflag 控制模式标志 */
#define OSAL_CSIZE    0x00000030  /* 字符大小掩码 */
#define OSAL_CS5      0x00000000  /* 5位 */
#define OSAL_CS6      0x00000010  /* 6位 */
#define OSAL_CS7      0x00000020  /* 7位 */
#define OSAL_CS8      0x00000030  /* 8位 */
#define OSAL_CSTOPB   0x00000040  /* 2个停止位 */
#define OSAL_CREAD    0x00000080  /* 启用接收 */
#define OSAL_PARENB   0x00000100  /* 启用奇偶校验 */
#define OSAL_PARODD   0x00000200  /* 奇校验 */
#define OSAL_HUPCL    0x00000400  /* 关闭时挂断 */
#define OSAL_CLOCAL   0x00000800  /* 忽略调制解调器状态线 */

/* c_lflag 本地模式标志 */
#define OSAL_ISIG     0x00000001  /* 启用信号 */
#define OSAL_ICANON   0x00000002  /* 规范模式 */
#define OSAL_ECHO     0x00000008  /* 回显输入字符 */
#define OSAL_ECHOE    0x00000010  /* 回显ERASE */
#define OSAL_ECHOK    0x00000020  /* 回显KILL */
#define OSAL_ECHONL   0x00000040  /* 回显NL */
#define OSAL_NOFLSH   0x00000080  /* 禁止在信号后刷新 */
#define OSAL_TOSTOP   0x00000100  /* 后台输出发送SIGTTOU */
#define OSAL_IEXTEN   0x00008000  /* 启用扩展功能 */

/* c_cc 控制字符索引 */
#define OSAL_VMIN     6   /* 非规范模式最小字符数 */
#define OSAL_VTIME    5   /* 非规范模式超时 */

/* tcsetattr optional_actions */
#define OSAL_TCSANOW   0  /* 立即生效 */
#define OSAL_TCSADRAIN 1  /* 输出完成后生效 */
#define OSAL_TCSAFLUSH 2  /* 输出完成后生效并刷新输入 */

/* tcflush queue_selector */
#define OSAL_TCIFLUSH  0  /* 刷新输入队列 */
#define OSAL_TCOFLUSH  1  /* 刷新输出队列 */
#define OSAL_TCIOFLUSH 2  /* 刷新输入和输出队列 */

/*
 * 标准文件描述符
 */
#define OSAL_STDIN_FILENO  0
#define OSAL_STDOUT_FILENO 1
#define OSAL_STDERR_FILENO 2

/*
 * 终端控制函数
 */
int32_t OSAL_tcgetattr(int32_t fd, osal_termios_t *termios_p);
int32_t OSAL_tcsetattr(int32_t fd, int32_t optional_actions, const osal_termios_t *termios_p);
int32_t OSAL_tcflush(int32_t fd, int32_t queue_selector);
int32_t OSAL_cfsetispeed(osal_termios_t *termios_p, uint32_t speed);
int32_t OSAL_cfsetospeed(osal_termios_t *termios_p, uint32_t speed);

#endif /* OSAL_TERMIOS_H */
