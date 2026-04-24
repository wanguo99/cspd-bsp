/************************************************************************
 * 以太网配置
 ************************************************************************/

#ifndef ETHERNET_CONFIG_H
#define ETHERNET_CONFIG_H

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

/* 以太网故障切换到UART的阈值(连续失败次数) */
#define ETHERNET_FAULT_THRESHOLD 5

#endif /* ETHERNET_CONFIG_H */
