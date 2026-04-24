/************************************************************************
 * 硬件配置库使用示例
 *
 * 演示如何使用硬件配置库
 ************************************************************************/

#include "hw_config_api.h"
#include "os_log.h"
#include "pdl_mcu.h"
#include <stdio.h>
#include <stdlib.h>

/* 外部函数声明 */
extern int32 HW_Config_RegisterAll(void);
extern const hw_board_config_t* HW_Config_SelectDefault(void);

/*===========================================================================
 * 示例1：基本使用
 *===========================================================================*/

static void example_basic_usage(void)
{
    int32 ret;
    const hw_board_config_t *board;

    OS_printf("\n=== Example 1: Basic Usage ===\n");

    /* 初始化配置库 */
    ret = HW_Config_Init();
    if (ret != OS_SUCCESS) {
        OS_printf("Failed to initialize config library\n");
        return;
    }

    /* 注册所有配置 */
    ret = HW_Config_RegisterAll();
    if (ret != OS_SUCCESS) {
        OS_printf("Failed to register configurations\n");
        return;
    }

    /* 选择默认配置 */
    board = HW_Config_SelectDefault();
    if (board == NULL) {
        OS_printf("Failed to select default configuration\n");
        return;
    }

    /* 打印配置信息 */
    HW_Config_Print(board);
}

/*===========================================================================
 * 示例2：查找特定配置
 *===========================================================================*/

static void example_find_config(void)
{
    const hw_board_config_t *board;

    OS_printf("\n=== Example 2: Find Specific Config ===\n");

    /* 查找TI AM625平台的H200 V1.0配置 */
    board = HW_Config_Find("ti/am625", "h200_payload", "v1.0");
    if (board != NULL) {
        OS_printf("Found config: %s/%s/%s\n",
                  board->platform, board->product, board->version);
        OS_printf("Description: %s\n", board->description);
    } else {
        OS_printf("Config not found\n");
    }

    /* 查找任意版本的H200配置 */
    board = HW_Config_Find("ti/am625", "h200_payload", NULL);
    if (board != NULL) {
        OS_printf("Found config (any version): %s/%s/%s\n",
                  board->platform, board->product, board->version);
    }
}

/*===========================================================================
 * 示例3：查询MCU配置并初始化
 *===========================================================================*/

static void example_mcu_init(void)
{
    const hw_board_config_t *board;
    const hw_mcu_config_t *mcu_cfg;
    mcu_handle_t mcu_handle;
    mcu_version_t version;
    int32 ret;

    OS_printf("\n=== Example 3: MCU Initialization ===\n");

    /* 获取当前板级配置 */
    board = HW_Config_GetBoard();
    if (board == NULL) {
        board = HW_Config_SelectDefault();
        if (board == NULL) {
            OS_printf("No board configuration available\n");
            return;
        }
    }

    /* 查找MCU配置 */
    mcu_cfg = HW_Config_FindMCU(board, "stm32_mcu");
    if (mcu_cfg == NULL) {
        OS_printf("MCU 'stm32_mcu' not found in configuration\n");
        return;
    }

    OS_printf("Found MCU: %s\n", mcu_cfg->name);
    OS_printf("  Interface: %s (%s)\n",
              mcu_cfg->interface->name,
              mcu_cfg->interface->type == HW_INTERFACE_UART ? "UART" :
              mcu_cfg->interface->type == HW_INTERFACE_CAN ? "CAN" :
              mcu_cfg->interface->type == HW_INTERFACE_I2C ? "I2C" : "Unknown");

    /* 将硬件配置转换为PDL层配置 */
    mcu_config_t pdl_mcu_cfg = {0};
    strncpy(pdl_mcu_cfg.name, mcu_cfg->name, sizeof(pdl_mcu_cfg.name) - 1);

    /* 根据接口类型设置PDL配置 */
    switch (mcu_cfg->interface->type) {
        case HW_INTERFACE_UART:
            pdl_mcu_cfg.interface = MCU_INTERFACE_SERIAL;
            pdl_mcu_cfg.serial.device = mcu_cfg->interface->config.uart.device;
            pdl_mcu_cfg.serial.baudrate = mcu_cfg->interface->config.uart.baudrate;
            pdl_mcu_cfg.serial.data_bits = mcu_cfg->interface->config.uart.data_bits;
            pdl_mcu_cfg.serial.stop_bits = mcu_cfg->interface->config.uart.stop_bits;
            pdl_mcu_cfg.serial.parity = (mcu_cfg->interface->config.uart.parity == UART_PARITY_NONE) ? 'N' :
                                        (mcu_cfg->interface->config.uart.parity == UART_PARITY_ODD) ? 'O' : 'E';
            break;

        case HW_INTERFACE_CAN:
            pdl_mcu_cfg.interface = MCU_INTERFACE_CAN;
            pdl_mcu_cfg.can.device = mcu_cfg->interface->config.can.device;
            pdl_mcu_cfg.can.bitrate = mcu_cfg->interface->config.can.bitrate;
            pdl_mcu_cfg.can.tx_id = mcu_cfg->interface->config.can.tx_id;
            pdl_mcu_cfg.can.rx_id = mcu_cfg->interface->config.can.rx_id;
            break;

        case HW_INTERFACE_I2C:
            pdl_mcu_cfg.interface = MCU_INTERFACE_I2C;
            /* I2C配置转换 */
            break;

        default:
            OS_printf("Unsupported interface type: %d\n", mcu_cfg->interface->type);
            return;
    }

    pdl_mcu_cfg.cmd_timeout_ms = mcu_cfg->cmd_timeout_ms;
    pdl_mcu_cfg.retry_count = mcu_cfg->retry_count;
    pdl_mcu_cfg.enable_crc = mcu_cfg->enable_crc;

    /* 初始化MCU */
    ret = PDL_MCU_Init(&pdl_mcu_cfg, &mcu_handle);
    if (ret != OS_SUCCESS) {
        OS_printf("Failed to initialize MCU\n");
        return;
    }

    /* 读取MCU版本 */
    ret = PDL_MCU_GetVersion(mcu_handle, &version);
    if (ret == OS_SUCCESS) {
        OS_printf("MCU Version: %d.%d.%d.%d (%s)\n",
                  version.major, version.minor, version.patch, version.build,
                  version.version_string);
    }

    /* 清理 */
    PDL_MCU_Deinit(mcu_handle);
}

/*===========================================================================
 * 示例4：列出所有配置
 *===========================================================================*/

static void example_list_configs(void)
{
    const hw_board_config_t *configs[32];
    uint32 count = 32;
    int32 ret;

    OS_printf("\n=== Example 4: List All Configs ===\n");

    ret = HW_Config_List(configs, &count);
    if (ret != OS_SUCCESS) {
        OS_printf("Failed to list configurations\n");
        return;
    }

    OS_printf("Total configurations: %d\n\n", count);

    for (uint32 i = 0; i < count; i++) {
        OS_printf("[%d] %s/%s/%s\n", i,
                  configs[i]->platform,
                  configs[i]->product,
                  configs[i]->version);
        OS_printf("    %s\n", configs[i]->description);
        OS_printf("    MCUs: %d, Sensors: %d, Interfaces: %d, Power Domains: %d\n",
                  configs[i]->mcu_count,
                  configs[i]->sensor_count,
                  configs[i]->interface_count,
                  configs[i]->power_domain_count);
        OS_printf("\n");
    }
}

/*===========================================================================
 * 主函数
 *===========================================================================*/

int main(int argc, char *argv[])
{
    /* 初始化日志系统 */
    OS_LogInit();

    OS_printf("\n");
    OS_printf("========================================\n");
    OS_printf("Hardware Configuration Library Examples\n");
    OS_printf("========================================\n");

    /* 运行示例 */
    example_basic_usage();
    example_find_config();
    example_list_configs();
    example_mcu_init();

    /* 清理 */
    HW_Config_Cleanup();
    OS_LogCleanup();

    return 0;
}
