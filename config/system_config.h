/************************************************************************
 * 系统配置文件
 *
 * 定义整个系统的配置参数
 ************************************************************************/

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/*
 * ============================================================================
 * 系统基础配置
 * ============================================================================
 */

/* 系统版本 */
#define SYSTEM_VERSION_MAJOR    1
#define SYSTEM_VERSION_MINOR    0
#define SYSTEM_VERSION_PATCH    0

/* 系统名称 */
#define SYSTEM_NAME             "CSPD-BSP"

/* 调试模式 */
#define DEBUG_MODE              1

/*
 * ============================================================================
 * OSAL配置
 * ============================================================================
 */

/* 任务栈大小 */
#define TASK_STACK_SIZE_SMALL   (32 * 1024)   /* 32KB */
#define TASK_STACK_SIZE_MEDIUM  (64 * 1024)   /* 64KB */
#define TASK_STACK_SIZE_LARGE   (128 * 1024)  /* 128KB */

/*
 * ============================================================================
 * CAN配置
 * ============================================================================
 */

/* CAN接口 */
#define CAN_INTERFACE           "can0"

/* CAN波特率 */
#define CAN_BAUDRATE            500000  /* 500Kbps */

/* CAN接收队列深度 */
#define CAN_RX_QUEUE_DEPTH      100

/* CAN发送队列深度 */
#define CAN_TX_QUEUE_DEPTH      100

/* CAN消息最大长度 */
#define CAN_MAX_DATA_LEN        8

/* CAN超时时间(ms) */
#define CAN_TIMEOUT_MS          1000

/*
 * ============================================================================
 * 以太网配置
 * ============================================================================
 */

/* 载荷IP地址 */
#define SERVER_IP_ADDRESS       "192.168.1.100"

/* IPMI端口 */
#define IPMI_PORT               623

/* Redfish端口 */
#define REDFISH_PORT            443

/* 以太网超时时间(ms) */
#define ETHERNET_TIMEOUT_MS     5000

/* 连接重试次数 */
#define ETHERNET_RETRY_COUNT    3

/* 连接重试间隔(ms) */
#define ETHERNET_RETRY_INTERVAL 1000

/*
 * ============================================================================
 * UART配置
 * ============================================================================
 */

/* UART设备 */
#define UART_DEVICE             "/dev/ttyS0"

/* UART波特率 */
#define UART_BAUDRATE           115200

/* UART数据位 */
#define UART_DATABITS           8

/* UART停止位 */
#define UART_STOPBITS           1

/* UART校验位 */
#define UART_PARITY             0  /* 0=无, 1=奇, 2=偶 */

/* UART超时时间(ms) */
#define UART_TIMEOUT_MS         2000

/*
 * ============================================================================
 * 消息队列配置
 * ============================================================================
 */

/* 消息队列深度 */
#define MSG_QUEUE_DEPTH         50

/* 消息最大长度 */
#define MSG_MAX_SIZE            512

/*
 * ============================================================================
 * 协议转换配置
 * ============================================================================
 */

/* 命令超时时间(ms) */
#define CMD_TIMEOUT_MS          10000

/* 最大重试次数 */
#define CMD_RETRY_COUNT         3

/* 命令缓存大小 */
#define CMD_CACHE_SIZE          100

/*
 * ============================================================================
 * 状态监控配置
 * ============================================================================
 */

/* 状态查询周期(ms) */
#define STATUS_QUERY_PERIOD_MS  5000

/* 状态缓存时间(ms) */
#define STATUS_CACHE_TIME_MS    10000

/* 异常检测阈值 */
#define FAULT_DETECT_THRESHOLD  3

/*
 * ============================================================================
 * 故障处理配置
 * ============================================================================
 */

/* 通信故障检测周期(ms) */
#define FAULT_CHECK_PERIOD_MS   1000

/* 以太网故障切换到UART的阈值(连续失败次数) */
#define ETHERNET_FAULT_THRESHOLD 5

/* 自动恢复尝试间隔(ms) */
#define AUTO_RECOVERY_INTERVAL_MS 30000

/*
 * ============================================================================
 * 日志配置
 * ============================================================================
 */

/* 日志级别 */
typedef enum
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/* 当前日志级别 */
#define LOG_LEVEL               LOG_LEVEL_INFO

/* 日志文件路径 */
#define LOG_FILE_PATH           "/var/log/cspd-bsp.log"

/* 日志文件最大大小(MB) */
#define LOG_FILE_MAX_SIZE_MB    10

/* 日志文件备份数量 */
#define LOG_FILE_BACKUP_COUNT   5

/*
 * ============================================================================
 * 看门狗配置
 * ============================================================================
 */

/* 软件看门狗使能 */
#define SOFTWARE_WATCHDOG_ENABLE 1

/* 软件看门狗超时时间(ms) */
#define SOFTWARE_WATCHDOG_TIMEOUT_MS 10000

/* 硬件看门狗设备 */
#define HARDWARE_WATCHDOG_DEVICE "/dev/watchdog"

/* 硬件看门狗超时时间(s) */
#define HARDWARE_WATCHDOG_TIMEOUT_S 30

/*
 * ============================================================================
 * 数据缓存配置
 * ============================================================================
 */

/* 缓存文件路径 */
#define CACHE_FILE_PATH         "/var/cache/cspd-bsp.cache"

/* 缓存最大条目数 */
#define CACHE_MAX_ENTRIES       1000

/* 缓存持久化周期(ms) */
#define CACHE_PERSIST_PERIOD_MS 60000

/*
 * ============================================================================
 * 任务优先级定义
 * ============================================================================
 */

/* 优先级: 数字越小优先级越高 */
#define PRIORITY_CRITICAL       10   /* CAN网关 */
#define PRIORITY_HIGH           50   /* 协议转换 */
#define PRIORITY_NORMAL         100  /* 状态监控 */
#define PRIORITY_LOW            150  /* 日志、缓存 */
#define PRIORITY_IDLE           200  /* 后台任务 */

#endif /* SYSTEM_CONFIG_H */
