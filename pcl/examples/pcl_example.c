/************************************************************************
 * 硬件配置库使用示例
 *
 * 演示如何使用硬件配置库
 ************************************************************************/

#include "pcl_api.h"
#include "osal.h"

/* 外部函数声明 */
extern int32_t PCL_RegisterAll(void);
extern const pcl_board_config_t* PCL_SelectDefault(void);

/*===========================================================================
 * 示例1：基本使用
 *===========================================================================*/

static void example_basic_usage(void)
{
    int32_t ret;
    const pcl_board_config_t *board;

    OSAL_Printf("\n=== Example 1: Basic Usage ===\n");

    /* 初始化配置库 */
    ret = PCL_Init();
    if (ret != OS_SUCCESS) {
        OSAL_Printf("Failed to initialize config library\n");
        return;
    }

    /* 注册所有配置 */
    ret = PCL_RegisterAll();
    if (ret != OS_SUCCESS) {
        OSAL_Printf("Failed to register configurations\n");
        return;
    }

    /* 选择默认配置 */
    board = PCL_SelectDefault();
    if (board == NULL) {
        OSAL_Printf("Failed to select default configuration\n");
        return;
    }

    /* 打印配置信息 */
    PCL_Print(board);
}

/*===========================================================================
 * 示例2：查找特定配置
 *===========================================================================*/

static void example_find_config(void)
{
    const pcl_board_config_t *board;

    OSAL_Printf("\n=== Example 2: Find Specific Config ===\n");

    /* 查找TI AM625平台的H200 V1.0配置 */
    board = PCL_Find("ti/am625", "h200_payload", "v1.0");
    if (board != NULL) {
        OSAL_Printf("Found config: %s/%s/%s\n",
                  board->platform, board->product, board->version);
        OSAL_Printf("Description: %s\n", board->description);
    } else {
        OSAL_Printf("Config not found\n");
    }

    /* 查找任意版本的H200配置 */
    board = PCL_Find("ti/am625", "h200_payload", NULL);
    if (board != NULL) {
        OSAL_Printf("Found config (any version): %s/%s/%s\n",
                  board->platform, board->product, board->version);
    }
}

/*===========================================================================
 * 示例3：查询MCU配置
 *===========================================================================*/

static void example_query_mcu_config(void)
{
    const pcl_board_config_t *board;
    const pcl_mcu_cfg_t *mcu_cfg;

    OSAL_Printf("\n=== Example 3: Query MCU Configuration ===\n");

    /* 获取当前板级配置 */
    board = PCL_GetBoard();
    if (board == NULL) {
        board = PCL_SelectDefault();
        if (board == NULL) {
            OSAL_Printf("No board configuration available\n");
            return;
        }
    }

    /* 查找MCU配置 */
    mcu_cfg = PCL_HW_FindMCU(board, "stm32_mcu");
    if (mcu_cfg == NULL) {
        OSAL_Printf("MCU 'stm32_mcu' not found in configuration\n");
        return;
    }

    /* 打印MCU配置信息 */
    OSAL_Printf("Found MCU: %s\n", mcu_cfg->name);
    OSAL_Printf("  Enabled: %s\n", mcu_cfg->enabled ? "Yes" : "No");
    OSAL_Printf("  Interface: %s\n",
              mcu_cfg->interface_type == PCL_HW_INTERFACE_UART ? "UART" :
              mcu_cfg->interface_type == PCL_HW_INTERFACE_CAN ? "CAN" :
              mcu_cfg->interface_type == PCL_HW_INTERFACE_I2C ? "I2C" :
              mcu_cfg->interface_type == PCL_HW_INTERFACE_SPI ? "SPI" : "Unknown");

    /* 根据接口类型打印详细配置 */
    switch (mcu_cfg->interface_type) {
        case PCL_HW_INTERFACE_UART:
            OSAL_Printf("  UART Config:\n");
            OSAL_Printf("    Device: %s\n", mcu_cfg->interface_cfg.uart.device);
            OSAL_Printf("    Baudrate: %u\n", mcu_cfg->interface_cfg.uart.baudrate);
            OSAL_Printf("    Data bits: %u\n", mcu_cfg->interface_cfg.uart.data_bits);
            OSAL_Printf("    Stop bits: %u\n", mcu_cfg->interface_cfg.uart.stop_bits);
            OSAL_Printf("    Parity: %c\n", mcu_cfg->interface_cfg.uart.parity);
            break;

        case PCL_HW_INTERFACE_CAN:
            OSAL_Printf("  CAN Config:\n");
            OSAL_Printf("    Device: %s\n", mcu_cfg->interface_cfg.can.device);
            OSAL_Printf("    Bitrate: %u\n", mcu_cfg->interface_cfg.can.bitrate);
            OSAL_Printf("    TX ID: 0x%X\n", mcu_cfg->interface_cfg.can.tx_id);
            OSAL_Printf("    RX ID: 0x%X\n", mcu_cfg->interface_cfg.can.rx_id);
            break;

        case PCL_HW_INTERFACE_I2C:
            OSAL_Printf("  I2C Config:\n");
            OSAL_Printf("    Device: %s\n", mcu_cfg->interface_cfg.i2c.device);
            OSAL_Printf("    Slave addr: 0x%X\n", mcu_cfg->interface_cfg.i2c.slave_addr);
            OSAL_Printf("    Speed: %u Hz\n", mcu_cfg->interface_cfg.i2c.speed_hz);
            break;

        default:
            OSAL_Printf("  Unsupported interface type: %d\n", mcu_cfg->interface_type);
            return;
    }

    /* 打印MCU特定配置 */
    OSAL_Printf("  MCU Settings:\n");
    OSAL_Printf("    Command timeout: %u ms\n", mcu_cfg->cmd_timeout_ms);
    OSAL_Printf("    Retry count: %u\n", mcu_cfg->retry_count);
    OSAL_Printf("    CRC enabled: %s\n", mcu_cfg->enable_crc ? "Yes" : "No");
}

/*===========================================================================
 * 示例4：列出所有配置
 *===========================================================================*/

static void example_list_configs(void)
{
    const pcl_board_config_t *configs[32];
    uint32_t count = 32;
    int32_t ret;

    OSAL_Printf("\n=== Example 4: List All Configs ===\n");

    ret = PCL_List(configs, &count);
    if (ret != OS_SUCCESS) {
        OSAL_Printf("Failed to list configurations\n");
        return;
    }

    OSAL_Printf("Total configurations: %d\n\n", count);

    for (uint32_t i = 0; i < count; i++) {
        OSAL_Printf("[%d] %s/%s/%s\n", i,
                  configs[i]->platform,
                  configs[i]->product,
                  configs[i]->version);
        OSAL_Printf("    %s\n", configs[i]->description);
        OSAL_Printf("    MCUs: %d, Sensors: %d, Interfaces: %d, Power Domains: %d\n",
                  configs[i]->mcu_count,
                  configs[i]->sensor_count,
                  configs[i]->interface_count,
                  configs[i]->power_domain_count);
        OSAL_Printf("\n");
    }
}

/*===========================================================================
 * 主函数
 *===========================================================================*/

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    OSAL_Printf("\n");
    OSAL_Printf("========================================\n");
    OSAL_Printf("Hardware Configuration Library Examples\n");
    OSAL_Printf("========================================\n");

    /* 运行示例 */
    example_basic_usage();
    example_find_config();
    example_list_configs();
    example_query_mcu_config();

    /* 清理 */
    PCL_Cleanup();

    return 0;
}
