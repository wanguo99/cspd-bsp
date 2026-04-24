/************************************************************************
 * 通用OS载荷通信服务
 *
 * 功能：
 * - 与不带BMC但运行Linux系统的载荷通信
 * - 支持SSH、HTTP API等通信方式
 * - 系统状态查询、进程管理、文件传输
 * - 远程命令执行
 ************************************************************************/

#ifndef PDL_PAYLOAD_OS_H
#define PDL_PAYLOAD_OS_H

#include "osa_types.h"

/*
 * OS载荷服务句柄
 */
typedef void* linux_payload_handle_t;

/*
 * 通信方式
 */
typedef enum
{
    LINUX_COMM_SSH = 0,      /* SSH协议 */
    LINUX_COMM_HTTP = 1,     /* HTTP API */
    LINUX_COMM_SERIAL = 2    /* 串口终端 */
} linux_comm_type_t;

/*
 * OS载荷配置
 */
typedef struct
{
    /* SSH配置 */
    struct {
        const char *ip_addr;      /* IP地址 */
        uint16 port;              /* 端口（默认22） */
        const char *username;     /* 用户名 */
        const char *password;     /* 密码或密钥路径 */
        uint32 timeout_ms;        /* 超时时间 */
    } ssh;

    /* HTTP API配置 */
    struct {
        const char *base_url;     /* API基础URL */
        const char *auth_token;   /* 认证令牌 */
        uint32 timeout_ms;        /* 超时时间 */
    } http;

    /* 串口配置 */
    struct {
        const char *device;       /* 串口设备 */
        uint32 baudrate;          /* 波特率 */
        uint32 timeout_ms;        /* 超时时间 */
    } serial;

    /* 服务配置 */
    linux_comm_type_t primary_comm;   /* 主通信方式 */
    bool auto_fallback;               /* 自动降级 */
    uint32 retry_count;               /* 重试次数 */
} linux_payload_config_t;

/*
 * 系统状态
 */
typedef struct
{
    bool online;              /* 在线状态 */
    float cpu_usage;          /* CPU使用率(%) */
    float mem_usage;          /* 内存使用率(%) */
    float disk_usage;         /* 磁盘使用率(%) */
    uint32 uptime_sec;        /* 运行时间(秒) */
    char kernel_version[64];  /* 内核版本 */
} linux_system_status_t;

/*
 * 进程信息
 */
typedef struct
{
    uint32 pid;               /* 进程ID */
    char name[64];            /* 进程名 */
    float cpu_usage;          /* CPU使用率 */
    uint32 mem_kb;            /* 内存使用(KB) */
    char state[16];           /* 状态 */
} linux_process_info_t;

/**
 * @brief 初始化OS载荷服务
 *
 * @param[in] config 配置参数
 * @param[out] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 LinuxPayloadPDL_Init(const linux_payload_config_t *config,
                        linux_payload_handle_t *handle);

/**
 * @brief 反初始化OS载荷服务
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_Deinit(linux_payload_handle_t handle);

/**
 * @brief 执行远程命令
 *
 * @param[in] handle 服务句柄
 * @param[in] command 命令字符串
 * @param[out] output 输出缓冲区
 * @param[in] output_size 缓冲区大小
 * @param[out] exit_code 退出码
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR_TIMEOUT 超时
 * @return OS_ERROR 失败
 */
int32 LinuxPayloadPDL_ExecuteCommand(linux_payload_handle_t handle,
                                  const char *command,
                                  char *output,
                                  uint32 output_size,
                                  int32 *exit_code);

/**
 * @brief 获取系统状态
 *
 * @param[in] handle 服务句柄
 * @param[out] status 系统状态
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_GetSystemStatus(linux_payload_handle_t handle,
                                   linux_system_status_t *status);

/**
 * @brief 获取进程列表
 *
 * @param[in] handle 服务句柄
 * @param[out] processes 进程信息数组
 * @param[in] max_count 数组大小
 * @param[out] actual_count 实际进程数
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_GetProcessList(linux_payload_handle_t handle,
                                  linux_process_info_t *processes,
                                  uint32 max_count,
                                  uint32 *actual_count);

/**
 * @brief 启动进程
 *
 * @param[in] handle 服务句柄
 * @param[in] command 启动命令
 * @param[out] pid 进程ID
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_StartProcess(linux_payload_handle_t handle,
                                const char *command,
                                uint32 *pid);

/**
 * @brief 停止进程
 *
 * @param[in] handle 服务句柄
 * @param[in] pid 进程ID
 * @param[in] force 是否强制终止
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_StopProcess(linux_payload_handle_t handle,
                               uint32 pid,
                               bool force);

/**
 * @brief 上传文件到载荷
 *
 * @param[in] handle 服务句柄
 * @param[in] local_path 本地文件路径
 * @param[in] remote_path 远程文件路径
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_UploadFile(linux_payload_handle_t handle,
                              const char *local_path,
                              const char *remote_path);

/**
 * @brief 从载荷下载文件
 *
 * @param[in] handle 服务句柄
 * @param[in] remote_path 远程文件路径
 * @param[in] local_path 本地文件路径
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_DownloadFile(linux_payload_handle_t handle,
                                const char *remote_path,
                                const char *local_path);

/**
 * @brief 系统重启
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_Reboot(linux_payload_handle_t handle);

/**
 * @brief 系统关机
 *
 * @param[in] handle 服务句柄
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_Shutdown(linux_payload_handle_t handle);

/**
 * @brief 切换通信方式
 *
 * @param[in] handle 服务句柄
 * @param[in] comm_type 通信方式
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_SwitchComm(linux_payload_handle_t handle,
                              linux_comm_type_t comm_type);

/**
 * @brief 检查连接状态
 *
 * @param[in] handle 服务句柄
 *
 * @return true 已连接
 * @return false 未连接
 */
bool LinuxPayloadPDL_IsConnected(linux_payload_handle_t handle);

/**
 * @brief 获取服务统计信息
 *
 * @param[in] handle 服务句柄
 * @param[out] cmd_count 命令总数
 * @param[out] success_count 成功数
 * @param[out] fail_count 失败数
 *
 * @return OS_SUCCESS 成功
 */
int32 LinuxPayloadPDL_GetStats(linux_payload_handle_t handle,
                            uint32 *cmd_count,
                            uint32 *success_count,
                            uint32 *fail_count);

#endif /* PDL_PAYLOAD_OS_H */
