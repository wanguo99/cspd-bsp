/************************************************************************
 * OSAL - termios系统调用封装实现（Linux）
 ************************************************************************/

#include "osal_termios.h"
#include <termios.h>
#include <unistd.h>
#include <string.h>

/*===========================================================================
 * 内部辅助函数 - 转换termios结构
 *===========================================================================*/

static void osal_to_native_termios(const osal_termios_t *osal_term, struct termios *native_term)
{
    memset(native_term, 0, sizeof(struct termios));

    native_term->c_iflag = osal_term->c_iflag;
    native_term->c_oflag = osal_term->c_oflag;
    native_term->c_cflag = osal_term->c_cflag;
    native_term->c_lflag = osal_term->c_lflag;
    native_term->c_line = osal_term->c_line;

    memcpy(native_term->c_cc, osal_term->c_cc, NCCS < OSAL_NCCS ? NCCS : OSAL_NCCS);
}

static void native_to_osal_termios(const struct termios *native_term, osal_termios_t *osal_term)
{
    memset(osal_term, 0, sizeof(osal_termios_t));

    osal_term->c_iflag = native_term->c_iflag;
    osal_term->c_oflag = native_term->c_oflag;
    osal_term->c_cflag = native_term->c_cflag;
    osal_term->c_lflag = native_term->c_lflag;
    osal_term->c_line = native_term->c_line;

    memcpy(osal_term->c_cc, native_term->c_cc, NCCS < OSAL_NCCS ? NCCS : OSAL_NCCS);

    osal_term->c_ispeed = cfgetispeed(native_term);
    osal_term->c_ospeed = cfgetospeed(native_term);
}

/*===========================================================================
 * termios函数实现
 *===========================================================================*/

int32 OSAL_tcgetattr(int32 fd, osal_termios_t *termios_p)
{
    struct termios native_term;
    int32 result = tcgetattr(fd, &native_term);

    if (result == 0) {
        native_to_osal_termios(&native_term, termios_p);
    }

    return result;
}

int32 OSAL_tcsetattr(int32 fd, int32 optional_actions, const osal_termios_t *termios_p)
{
    struct termios native_term;
    osal_to_native_termios(termios_p, &native_term);

    cfsetispeed(&native_term, termios_p->c_ispeed);
    cfsetospeed(&native_term, termios_p->c_ospeed);

    return tcsetattr(fd, optional_actions, &native_term);
}

int32 OSAL_tcflush(int32 fd, int32 queue_selector)
{
    return tcflush(fd, queue_selector);
}

int32 OSAL_tcflow(int32 fd, int32 action)
{
    return tcflow(fd, action);
}

int32 OSAL_tcsendbreak(int32 fd, int32 duration)
{
    return tcsendbreak(fd, duration);
}

int32 OSAL_tcdrain(int32 fd)
{
    return tcdrain(fd);
}

int32 OSAL_cfsetispeed(osal_termios_t *termios_p, uint32 speed)
{
    termios_p->c_ispeed = speed;
    return 0;
}

int32 OSAL_cfsetospeed(osal_termios_t *termios_p, uint32 speed)
{
    termios_p->c_ospeed = speed;
    return 0;
}

uint32 OSAL_cfgetispeed(const osal_termios_t *termios_p)
{
    return termios_p->c_ispeed;
}

uint32 OSAL_cfgetospeed(const osal_termios_t *termios_p)
{
    return termios_p->c_ospeed;
}
