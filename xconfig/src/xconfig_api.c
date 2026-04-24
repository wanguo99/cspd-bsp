/************************************************************************
 * 硬件配置库API实现
 *
 * 命名规范：
 * - XCONFIG_*       - 通用接口
 * - XCONFIG_HW_*    - 硬件配置接口
 * - XCONFIG_APP_*   - APP配置接口
 ************************************************************************/

#include "xconfig_api.h"
#include "osal_log.h"
#include "osal.h"
#include <string.h>
#include <stdio.h>

/*===========================================================================
 * 内部数据结构
 *===========================================================================*/

#define MAX_BOARD_CONFIGS 32

typedef struct {
    const xconfig_board_config_t *configs[MAX_BOARD_CONFIGS];
    uint32 count;
    const xconfig_board_config_t *current;
} xconfig_registry_t;

static xconfig_registry_t g_registry = {0};
static bool g_initialized = false;

/*===========================================================================
 * 配置库初始化
 *===========================================================================*/

int32 XCONFIG_Init(void)
{
    if (g_initialized) {
        return OS_SUCCESS;
    }

    memset(&g_registry, 0, sizeof(g_registry));
    g_initialized = true;

    LOG_INFO("XCONFIG", "Hardware configuration library initialized");
    return OS_SUCCESS;
}

void XCONFIG_Cleanup(void)
{
    if (!g_initialized) {
        return;
    }

    memset(&g_registry, 0, sizeof(g_registry));
    g_initialized = false;

    LOG_INFO("XCONFIG", "Hardware configuration library cleaned up");
}

/*===========================================================================
 * 板级配置注册和查询
 *===========================================================================*/

int32 XCONFIG_Register(const xconfig_board_config_t *config)
{
    if (!g_initialized) {
        LOG_ERROR("XCONFIG", "Library not initialized");
        return OS_ERROR;
    }

    if (config == NULL) {
        LOG_ERROR("XCONFIG", "Invalid config pointer");
        return OS_ERROR;
    }

    if (g_registry.count >= MAX_BOARD_CONFIGS) {
        LOG_ERROR("XCONFIG", "Registry full (max %d configs)", MAX_BOARD_CONFIGS);
        return OS_ERROR;
    }

    /* 验证配置 */
    if (XCONFIG_Validate(config) != OS_SUCCESS) {
        LOG_ERROR("XCONFIG", "Config validation failed: %s/%s/%s",
                  config->platform, config->product, config->version);
        return OS_ERROR;
    }

    /* 检查重复 */
    for (uint32 i = 0; i < g_registry.count; i++) {
        const xconfig_board_config_t *existing = g_registry.configs[i];
        if (strcmp(existing->platform, config->platform) == 0 &&
            strcmp(existing->product, config->product) == 0 &&
            strcmp(existing->version, config->version) == 0) {
            LOG_WARN("XCONFIG", "Config already registered: %s/%s/%s",
                     config->platform, config->product, config->version);
            return OS_ERROR;
        }
    }

    /* 注册配置 */
    g_registry.configs[g_registry.count++] = config;

    LOG_INFO("XCONFIG", "Registered config: %s/%s/%s - %s",
             config->platform, config->product, config->version,
             config->description);

    return OS_SUCCESS;
}

const xconfig_board_config_t* XCONFIG_GetBoard(void)
{
    if (!g_initialized) {
        return NULL;
    }
    return g_registry.current;
}

const xconfig_board_config_t* XCONFIG_Find(const char *platform,
                                       const char *product,
                                       const char *version)
{
    if (!g_initialized || platform == NULL || product == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < g_registry.count; i++) {
        const xconfig_board_config_t *config = g_registry.configs[i];

        if (strcmp(config->platform, platform) != 0) {
            continue;
        }

        if (strcmp(config->product, product) != 0) {
            continue;
        }

        /* 如果指定了版本，则精确匹配；否则返回第一个匹配的 */
        if (version == NULL || strcmp(config->version, version) == 0) {
            return config;
        }
    }

    return NULL;
}

int32 XCONFIG_List(const xconfig_board_config_t **configs, uint32 *count)
{
    if (!g_initialized || configs == NULL || count == NULL) {
        return OS_ERROR;
    }

    uint32 max_count = *count;
    uint32 actual_count = (g_registry.count < max_count) ? g_registry.count : max_count;

    for (uint32 i = 0; i < actual_count; i++) {
        configs[i] = g_registry.configs[i];
    }

    *count = actual_count;
    return OS_SUCCESS;
}

/*===========================================================================
 * 硬件外设配置查询接口（XCONFIG_HW_*）
 *===========================================================================*/

const xconfig_mcu_cfg_t* XCONFIG_HW_FindMCU(const xconfig_board_config_t *board,
                                        const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->mcu_count; i++) {
        if (strcmp(board->mcus[i]->name, name) == 0) {
            return board->mcus[i];
        }
    }

    return NULL;
}

const xconfig_mcu_cfg_t* XCONFIG_HW_GetMCU(const xconfig_board_config_t *board,
                                       uint32 id)
{
    if (board == NULL || id >= board->mcu_count) {
        return NULL;
    }

    return board->mcus[id];
}

const xconfig_bmc_cfg_t* XCONFIG_HW_FindBMC(const xconfig_board_config_t *board,
                                        const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->bmc_count; i++) {
        if (strcmp(board->bmcs[i]->name, name) == 0) {
            return board->bmcs[i];
        }
    }

    return NULL;
}

const xconfig_bmc_cfg_t* XCONFIG_HW_GetBMC(const xconfig_board_config_t *board,
                                       uint32 id)
{
    if (board == NULL || id >= board->bmc_count) {
        return NULL;
    }

    return board->bmcs[id];
}

const xconfig_satellite_cfg_t* XCONFIG_HW_FindSatellite(const xconfig_board_config_t *board,
                                                     const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->satellite_count; i++) {
        if (strcmp(board->satellites[i]->name, name) == 0) {
            return board->satellites[i];
        }
    }

    return NULL;
}

const xconfig_satellite_cfg_t* XCONFIG_HW_GetSatellite(const xconfig_board_config_t *board,
                                                    uint32 id)
{
    if (board == NULL || id >= board->satellite_count) {
        return NULL;
    }

    return board->satellites[id];
}

const xconfig_sensor_cfg_t* XCONFIG_HW_FindSensor(const xconfig_board_config_t *board,
                                              const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->sensor_count; i++) {
        if (strcmp(board->sensors[i]->name, name) == 0) {
            return board->sensors[i];
        }
    }

    return NULL;
}

const xconfig_sensor_cfg_t* XCONFIG_HW_GetSensor(const xconfig_board_config_t *board,
                                             uint32 id)
{
    if (board == NULL || id >= board->sensor_count) {
        return NULL;
    }

    return board->sensors[id];
}

const xconfig_storage_cfg_t* XCONFIG_HW_FindStorage(const xconfig_board_config_t *board,
                                                 const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->storage_count; i++) {
        if (strcmp(board->storages[i]->name, name) == 0) {
            return board->storages[i];
        }
    }

    return NULL;
}

const xconfig_storage_cfg_t* XCONFIG_HW_GetStorage(const xconfig_board_config_t *board,
                                                uint32 id)
{
    if (board == NULL || id >= board->storage_count) {
        return NULL;
    }

    return board->storages[id];
}

const xconfig_power_domain_t* XCONFIG_HW_FindPowerDomain(const xconfig_board_config_t *board,
                                                      const char *name)
{
    if (board == NULL || name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->power_domain_count; i++) {
        if (strcmp(board->power_domains[i]->name, name) == 0) {
            return board->power_domains[i];
        }
    }

    return NULL;
}

/*===========================================================================
 * APP配置查询接口（XCONFIG_APP_*）
 *===========================================================================*/

const xconfig_app_config_t* XCONFIG_APP_Find(const xconfig_board_config_t *board,
                                         const char *app_name)
{
    if (board == NULL || app_name == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < board->app_count; i++) {
        if (strcmp(board->apps[i]->app_name, app_name) == 0) {
            return board->apps[i];
        }
    }

    return NULL;
}

const xconfig_app_device_mapping_t* XCONFIG_APP_FindDevice(const xconfig_app_config_t *app,
                                                        const char *function)
{
    if (app == NULL || function == NULL) {
        return NULL;
    }

    for (uint32 i = 0; i < app->mapping_count; i++) {
        if (strcmp(app->device_mappings[i].function, function) == 0) {
            return &app->device_mappings[i];
        }
    }

    return NULL;
}

const void* XCONFIG_APP_GetDeviceByMapping(const xconfig_board_config_t *board,
                                             const xconfig_app_device_mapping_t *mapping)
{
    if (board == NULL || mapping == NULL) {
        return NULL;
    }

    switch (mapping->device_type) {
        case XCONFIG_DEV_MCU:
            return XCONFIG_HW_GetMCU(board, mapping->device_id);

        case XCONFIG_DEV_BMC:
            return XCONFIG_HW_GetBMC(board, mapping->device_id);

        case XCONFIG_DEV_SATELLITE:
            return XCONFIG_HW_GetSatellite(board, mapping->device_id);

        case XCONFIG_DEV_SENSOR:
            return XCONFIG_HW_GetSensor(board, mapping->device_id);

        case XCONFIG_DEV_STORAGE:
            return XCONFIG_HW_GetStorage(board, mapping->device_id);

        default:
            return NULL;
    }
}

/*===========================================================================
 * 配置验证
 *===========================================================================*/

int32 XCONFIG_Validate(const xconfig_board_config_t *config)
{
    if (config == NULL) {
        LOG_ERROR("XCONFIG", "Config is NULL");
        return OS_ERROR;
    }

    /* 验证基本字段 */
    if (config->platform == NULL || strlen(config->platform) == 0) {
        LOG_ERROR("XCONFIG", "Invalid platform name");
        return OS_ERROR;
    }

    if (config->product == NULL || strlen(config->product) == 0) {
        LOG_ERROR("XCONFIG", "Invalid product name");
        return OS_ERROR;
    }

    if (config->version == NULL || strlen(config->version) == 0) {
        LOG_ERROR("XCONFIG", "Invalid version");
        return OS_ERROR;
    }

    /* 验证MCU配置 */
    if (config->mcu_count > 0 && config->mcus == NULL) {
        LOG_ERROR("XCONFIG", "MCU count > 0 but mcus is NULL");
        return OS_ERROR;
    }

    for (uint32 i = 0; i < config->mcu_count; i++) {
        if (config->mcus[i] == NULL) {
            LOG_ERROR("XCONFIG", "MCU[%d] is NULL", i);
            return OS_ERROR;
        }
        if (config->mcus[i]->name == NULL) {
            LOG_ERROR("XCONFIG", "MCU[%d] name is NULL", i);
            return OS_ERROR;
        }
    }

    /* 验证BMC配置 */
    if (config->bmc_count > 0 && config->bmcs == NULL) {
        LOG_ERROR("XCONFIG", "BMC count > 0 but bmcs is NULL");
        return OS_ERROR;
    }

    /* 验证卫星平台配置 */
    if (config->satellite_count > 0 && config->satellites == NULL) {
        LOG_ERROR("XCONFIG", "Satellite count > 0 but satellites is NULL");
        return OS_ERROR;
    }

    /* 验证传感器配置 */
    if (config->sensor_count > 0 && config->sensors == NULL) {
        LOG_ERROR("XCONFIG", "Sensor count > 0 but sensors is NULL");
        return OS_ERROR;
    }

    /* 验证存储设备配置 */
    if (config->storage_count > 0 && config->storages == NULL) {
        LOG_ERROR("XCONFIG", "Storage count > 0 but storages is NULL");
        return OS_ERROR;
    }

    /* 验证电源域配置 */
    if (config->power_domain_count > 0 && config->power_domains == NULL) {
        LOG_ERROR("XCONFIG", "Power domain count > 0 but power_domains is NULL");
        return OS_ERROR;
    }

    /* 验证APP配置 */
    if (config->app_count > 0 && config->apps == NULL) {
        LOG_ERROR("XCONFIG", "APP count > 0 but apps is NULL");
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

void XCONFIG_Print(const xconfig_board_config_t *config)
{
    if (config == NULL) {
        printf("Config is NULL\n");
        return;
    }

    printf("\n========================================\n");
    printf("Board Configuration\n");
    printf("========================================\n");
    printf("Platform:    %s\n", config->platform);
    printf("Product:     %s\n", config->product);
    printf("Version:     %s\n", config->version);
    printf("Description: %s\n", config->description);
    printf("\n");

    /* 打印MCU外设配置 */
    printf("MCU Peripherals: %d\n", config->mcu_count);
    for (uint32 i = 0; i < config->mcu_count; i++) {
        const xconfig_mcu_cfg_t *mcu = config->mcus[i];
        printf("  [%d] %s - %s\n", i, mcu->name,
                  mcu->enabled ? "Enabled" : "Disabled");
        printf("      Interface: %s\n",
                  mcu->interface_type == XCONFIG_INTERFACE_CAN ? "CAN" :
                  mcu->interface_type == XCONFIG_INTERFACE_UART ? "UART" :
                  mcu->interface_type == XCONFIG_INTERFACE_I2C ? "I2C" : "Unknown");
    }
    printf("\n");

    /* 打印BMC外设配置 */
    printf("BMC Peripherals: %d\n", config->bmc_count);
    for (uint32 i = 0; i < config->bmc_count; i++) {
        const xconfig_bmc_cfg_t *bmc = config->bmcs[i];
        printf("  [%d] %s - %s\n", i, bmc->name,
                  bmc->enabled ? "Enabled" : "Disabled");
    }
    printf("\n");

    /* 打印卫星平台接口配置 */
    printf("Satellite Interfaces: %d\n", config->satellite_count);
    for (uint32 i = 0; i < config->satellite_count; i++) {
        const xconfig_satellite_cfg_t *sat = config->satellites[i];
        printf("  [%d] %s - %s\n", i, sat->name,
                  sat->enabled ? "Enabled" : "Disabled");
    }
    printf("\n");

    /* 打印传感器外设配置 */
    printf("Sensor Peripherals: %d\n", config->sensor_count);
    for (uint32 i = 0; i < config->sensor_count; i++) {
        const xconfig_sensor_cfg_t *sensor = config->sensors[i];
        printf("  [%d] %s - %s\n", i, sensor->name,
                  sensor->enabled ? "Enabled" : "Disabled");
    }
    printf("\n");

    /* 打印存储设备配置 */
    printf("Storage Devices: %d\n", config->storage_count);
    for (uint32 i = 0; i < config->storage_count; i++) {
        const xconfig_storage_cfg_t *storage = config->storages[i];
        printf("  [%d] %s - %s\n", i, storage->name,
                  storage->enabled ? "Enabled" : "Disabled");
    }
    printf("\n");

    /* 打印APP配置 */
    printf("APP Configurations: %d\n", config->app_count);
    for (uint32 i = 0; i < config->app_count; i++) {
        const xconfig_app_config_t *app = config->apps[i];
        printf("  [%d] %s - %s\n", i, app->app_name, app->description);
        printf("      Device Mappings: %d\n", app->mapping_count);
        for (uint32 j = 0; j < app->mapping_count; j++) {
            const xconfig_app_device_mapping_t *mapping = &app->device_mappings[j];
            printf("        - %s: %s[%d] %s\n",
                      mapping->function,
                      mapping->device_type == XCONFIG_DEV_MCU ? "MCU" :
                      mapping->device_type == XCONFIG_DEV_BMC ? "BMC" :
                      mapping->device_type == XCONFIG_DEV_SATELLITE ? "SATELLITE" :
                      mapping->device_type == XCONFIG_DEV_SENSOR ? "SENSOR" :
                      mapping->device_type == XCONFIG_DEV_STORAGE ? "STORAGE" : "Unknown",
                      mapping->device_id,
                      mapping->required ? "(Required)" : "(Optional)");
        }
    }

    printf("========================================\n\n");
}
