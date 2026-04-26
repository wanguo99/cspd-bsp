/************************************************************************
 * 错误处理API
 ************************************************************************/

#ifndef OSAPI_ERROR_H
#define OSAPI_ERROR_H

#include "osal_types.h"

/**
 * @brief 获取错误描述字符串
 *
 * @param[in] error_num 错误码
 *
 * @return 错误描述字符串
 */
const str_t *OS_GetErrorName(int32 error_num);

/**
 * @brief 打印调试信息
 *
 * @param[in] format 格式化字符串
 * @param[in] ...    可变参数
 */
void OSAL_Printf(const str_t *format, ...);

#endif /* OSAPI_ERROR_H */
