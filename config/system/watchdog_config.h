/************************************************************************
 * 看门狗配置
 ************************************************************************/

#ifndef WATCHDOG_CONFIG_H
#define WATCHDOG_CONFIG_H

/* 软件看门狗使能 */
#define SOFTWARE_WATCHDOG_ENABLE 1

/* 软件看门狗超时时间(ms) */
#define SOFTWARE_WATCHDOG_TIMEOUT_MS 10000

/* 硬件看门狗设备 */
#define HARDWARE_WATCHDOG_DEVICE "/dev/watchdog"

/* 硬件看门狗超时时间(s) */
#define HARDWARE_WATCHDOG_TIMEOUT_S 30

/* 故障检测配置 */
#define FAULT_CHECK_PERIOD_MS   1000
#define FAULT_DETECT_THRESHOLD  3
#define AUTO_RECOVERY_INTERVAL_MS 30000

#endif /* WATCHDOG_CONFIG_H */
