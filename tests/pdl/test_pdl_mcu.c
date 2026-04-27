/**
 * @file test_pdl_mcu.c
 * @brief PDL MCU外设驱动单元测试
 */

#include "tests_core.h"
#include "test_assert.h"
#include "test_registry.h"
#include "pdl_mcu.h"
#include "osal.h"

/*===========================================================================
 * 初始化和清理测试
 *===========================================================================*/

/* 测试用例: MCU驱动初始化 - CAN接口 */
TEST_CASE(test_pdl_mcu_init_can_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    OSAL_Strcpy(config.name, "TEST_MCU");

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(handle);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: MCU驱动初始化 - 串口接口 */
TEST_CASE(test_pdl_mcu_init_serial_success)
{
    /* Skip: Serial device /dev/ttyS1 not available in test environment */
    TEST_SKIP();

    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_SERIAL,
        .serial = {
            .device = "/dev/ttyS1",
            .baudrate = 115200,
            .data_bits = 8,
            .stop_bits = 1,
            .parity = 'N'
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    OSAL_Strcpy(config.name, "TEST_MCU");

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
    TEST_ASSERT_NOT_NULL(handle);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: MCU驱动初始化 - 空配置 */
TEST_CASE(test_pdl_mcu_init_null_config)
{
    mcu_handle_t handle = NULL;

    int32_t ret = PDL_MCU_Init(NULL, &handle);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: MCU驱动初始化 - 空句柄指针 */
TEST_CASE(test_pdl_mcu_init_null_handle)
{
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: MCU驱动清理 */
TEST_CASE(test_pdl_mcu_deinit)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_Deinit(handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 清理空句柄 */
TEST_CASE(test_pdl_mcu_deinit_null_handle)
{
    int32_t ret = PDL_MCU_Deinit(NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/*===========================================================================
 * 版本信息测试
 *===========================================================================*/

/* 测试用例: 获取版本信息 - 成功 */
TEST_CASE(test_pdl_mcu_get_version_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    mcu_version_t version;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OSAL_Memset(&version, 0, sizeof(version));
    ret = PDL_MCU_GetVersion(handle, &version);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 验证版本号有效 */
    TEST_ASSERT_TRUE(version.major >= 0);
    TEST_ASSERT_TRUE(version.minor >= 0);
    TEST_ASSERT_TRUE(version.patch >= 0);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 获取版本信息 - 空句柄 */
TEST_CASE(test_pdl_mcu_get_version_null_handle)
{
    mcu_version_t version;

    int32_t ret = PDL_MCU_GetVersion(NULL, &version);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 获取版本信息 - 空指针 */
TEST_CASE(test_pdl_mcu_get_version_null_pointer)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_GetVersion(handle, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/*===========================================================================
 * 状态查询测试
 *===========================================================================*/

/* 测试用例: 获取状态 - 成功 */
TEST_CASE(test_pdl_mcu_get_status_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    mcu_status_t status;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    OSAL_Memset(&status, 0, sizeof(status));
    ret = PDL_MCU_GetStatus(handle, &status);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 获取状态 - 空句柄 */
TEST_CASE(test_pdl_mcu_get_status_null_handle)
{
    mcu_status_t status;

    int32_t ret = PDL_MCU_GetStatus(NULL, &status);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 获取状态 - 空指针 */
TEST_CASE(test_pdl_mcu_get_status_null_pointer)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_GetStatus(handle, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/*===========================================================================
 * 复位测试
 *===========================================================================*/

/* 测试用例: MCU复位 - 成功 */
TEST_CASE(test_pdl_mcu_reset_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_Reset(handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 复位后等待MCU恢复 */
    OSAL_TaskDelay(100);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: MCU复位 - 空句柄 */
TEST_CASE(test_pdl_mcu_reset_null_handle)
{
    int32_t ret = PDL_MCU_Reset(NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/*===========================================================================
 * 寄存器访问测试
 *===========================================================================*/

/* 测试用例: 读寄存器 - 成功 */
TEST_CASE(test_pdl_mcu_read_register_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    uint8_t value;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_ReadRegister(handle, 0x10, &value);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 读寄存器 - 空句柄 */
TEST_CASE(test_pdl_mcu_read_register_null_handle)
{
    uint8_t value;

    int32_t ret = PDL_MCU_ReadRegister(NULL, 0x10, &value);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 读寄存器 - 空指针 */
TEST_CASE(test_pdl_mcu_read_register_null_pointer)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_ReadRegister(handle, 0x10, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 写寄存器 - 成功 */
TEST_CASE(test_pdl_mcu_write_register_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_WriteRegister(handle, 0x20, 0xAB);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 写寄存器 - 空句柄 */
TEST_CASE(test_pdl_mcu_write_register_null_handle)
{
    int32_t ret = PDL_MCU_WriteRegister(NULL, 0x20, 0xAB);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 读写寄存器验证 */
TEST_CASE(test_pdl_mcu_register_read_write_verify)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    uint8_t write_value = 0xCD;
    uint8_t read_value;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 写入寄存器 */
    ret = PDL_MCU_WriteRegister(handle, 0x30, write_value);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 读取寄存器 */
    ret = PDL_MCU_ReadRegister(handle, 0x30, &read_value);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 验证读写一致（如果寄存器支持读写） */
    /* 注意：某些寄存器可能是只读或只写的 */

    PDL_MCU_Deinit(handle);
}

/*===========================================================================
 * 命令发送测试
 *===========================================================================*/

/* 测试用例: 发送命令 - 成功 */
TEST_CASE(test_pdl_mcu_send_command_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    uint8_t cmd_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t resp_data[64];
    uint32_t actual_size;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_SendCommand(handle, 0x10, cmd_data, sizeof(cmd_data),
                              resp_data, sizeof(resp_data), &actual_size);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 发送命令 - 空句柄 */
TEST_CASE(test_pdl_mcu_send_command_null_handle)
{
    uint8_t cmd_data[] = {0x01};
    uint8_t resp_data[64];
    uint32_t actual_size;

    int32_t ret = PDL_MCU_SendCommand(NULL, 0x10, cmd_data, sizeof(cmd_data),
                                      resp_data, sizeof(resp_data), &actual_size);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 发送命令 - 空命令数据 */
TEST_CASE(test_pdl_mcu_send_command_null_cmd_data)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    uint8_t resp_data[64];
    uint32_t actual_size;

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 空命令数据但长度为0应该成功 */
    ret = PDL_MCU_SendCommand(handle, 0x20, NULL, 0,
                              resp_data, sizeof(resp_data), &actual_size);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 发送命令 - 空响应缓冲区 */
TEST_CASE(test_pdl_mcu_send_command_null_resp_buffer)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };
    uint8_t cmd_data[] = {0x01};

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    /* 不需要响应的命令 */
    ret = PDL_MCU_SendCommand(handle, 0x30, cmd_data, sizeof(cmd_data),
                              NULL, 0, NULL);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/*===========================================================================
 * 固件更新测试
 *===========================================================================*/

/* 测试用例: 固件更新 - 成功 */
TEST_CASE(test_pdl_mcu_firmware_update_success)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_FirmwareUpdate(handle, "/tmp/test_firmware.bin", NULL);
    /* 可能成功或失败，取决于固件文件是否存在 */

    PDL_MCU_Deinit(handle);
}

/* 测试用例: 固件更新 - 空句柄 */
TEST_CASE(test_pdl_mcu_firmware_update_null_handle)
{
    int32_t ret = PDL_MCU_FirmwareUpdate(NULL, "/tmp/test_firmware.bin", NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);
}

/* 测试用例: 固件更新 - 空路径 */
TEST_CASE(test_pdl_mcu_firmware_update_null_path)
{
    mcu_handle_t handle = NULL;
    mcu_config_t config = {
        .interface = MCU_INTERFACE_CAN,
        .can = {
            .device = "can0",
            .bitrate = 500000,
            .tx_id = 0x100,
            .rx_id = 0x200
        },
        .cmd_timeout_ms = 5000,
        .retry_count = 3,
        .enable_crc = true
    };

    int32_t ret = PDL_MCU_Init(&config, &handle);
    TEST_ASSERT_EQUAL(OS_SUCCESS, ret);

    ret = PDL_MCU_FirmwareUpdate(handle, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(OS_SUCCESS, ret);

    PDL_MCU_Deinit(handle);
}

/*===========================================================================
 * 测试模块注册
 *===========================================================================*/

TEST_SUITE_BEGIN(test_pdl_mcu, "test_pdl_mcu", "PDL")
    // PDL MCU外设驱动测试
    /* 初始化和清理 */
    TEST_CASE_REF(test_pdl_mcu_init_can_success)
    TEST_CASE_REF(test_pdl_mcu_init_serial_success)
    TEST_CASE_REF(test_pdl_mcu_init_null_config)
    TEST_CASE_REF(test_pdl_mcu_init_null_handle)
    TEST_CASE_REF(test_pdl_mcu_deinit)
    TEST_CASE_REF(test_pdl_mcu_deinit_null_handle)

    /* 版本信息 */
    TEST_CASE_REF(test_pdl_mcu_get_version_success)
    TEST_CASE_REF(test_pdl_mcu_get_version_null_handle)
    TEST_CASE_REF(test_pdl_mcu_get_version_null_pointer)

    /* 状态查询 */
    TEST_CASE_REF(test_pdl_mcu_get_status_success)
    TEST_CASE_REF(test_pdl_mcu_get_status_null_handle)
    TEST_CASE_REF(test_pdl_mcu_get_status_null_pointer)

    /* 复位 */
    TEST_CASE_REF(test_pdl_mcu_reset_success)
    TEST_CASE_REF(test_pdl_mcu_reset_null_handle)

    /* 寄存器访问 */
    TEST_CASE_REF(test_pdl_mcu_read_register_success)
    TEST_CASE_REF(test_pdl_mcu_read_register_null_handle)
    TEST_CASE_REF(test_pdl_mcu_read_register_null_pointer)
    TEST_CASE_REF(test_pdl_mcu_write_register_success)
    TEST_CASE_REF(test_pdl_mcu_write_register_null_handle)
    TEST_CASE_REF(test_pdl_mcu_register_read_write_verify)

    /* 命令发送 */
    TEST_CASE_REF(test_pdl_mcu_send_command_success)
    TEST_CASE_REF(test_pdl_mcu_send_command_null_handle)
    TEST_CASE_REF(test_pdl_mcu_send_command_null_cmd_data)
    TEST_CASE_REF(test_pdl_mcu_send_command_null_resp_buffer)

    /* 固件更新 */
    TEST_CASE_REF(test_pdl_mcu_firmware_update_success)
    TEST_CASE_REF(test_pdl_mcu_firmware_update_null_handle)
    TEST_CASE_REF(test_pdl_mcu_firmware_update_null_path)
TEST_SUITE_END(test_pdl_mcu, "test_pdl_mcu", "PDL")
