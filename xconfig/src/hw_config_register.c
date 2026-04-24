/************************************************************************
 * 硬件配置自动注册
 *
 * 功能：
 * - 自动注册所有平台的硬件配置
 * - 在系统启动时调用
 ************************************************************************/

#include "hw_config_api.h"
#include "osal_log.h"
#include <stdlib.h>

/*===========================================================================
 * 外部配置声明
 *===========================================================================*/

/* TI AM625平台 - H200载荷板 */
extern const xconfig_board_config_t xconfig_h200_base;
extern const xconfig_board_config_t xconfig_h200_v1;
extern const xconfig_board_config_t xconfig_h200_v2;

/* 其他平台配置可以在这里添加 */
/* extern const xconfig_board_config_t hw_config_xxx; */

/*===========================================================================
 * 配置注册表
 *===========================================================================*/

static const xconfig_board_config_t* g_all_configs[] = {
    &xconfig_h200_base,
    &xconfig_h200_v1,
    &xconfig_h200_v2,
    /* 在这里添加新的配置 */
};

#define CONFIG_COUNT (sizeof(g_all_configs) / sizeof(g_all_configs[0]))

/*===========================================================================
 * 注册函数
 *===========================================================================*/

/**
 * @brief 注册所有硬件配置
 *
 * @return OS_SUCCESS 成功
 * @return OS_ERROR 失败
 */
int32 XCONFIG_RegisterAll(void)
{
    int32 ret;
    uint32 success_count = 0;

    LOG_INFO("XCONFIG", "Registering %d hardware configurations...", CONFIG_COUNT);

    for (uint32 i = 0; i < CONFIG_COUNT; i++) {
        ret = XCONFIG_Register(g_all_configs[i]);
        if (ret == OS_SUCCESS) {
            success_count++;
        } else {
            LOG_ERROR("XCONFIG", "Failed to register config[%d]: %s/%s/%s",
                      i,
                      g_all_configs[i]->platform,
                      g_all_configs[i]->product,
                      g_all_configs[i]->version);
        }
    }

    LOG_INFO("XCONFIG", "Registered %d/%d configurations successfully",
             success_count, CONFIG_COUNT);

    return (success_count > 0) ? OS_SUCCESS : OS_ERROR;
}

/**
 * @brief 根据环境变量或编译选项选择默认配置
 *
 * 优先级：
 * 1. 环境变量 XCONFIG_PLATFORM, XCONFIG_PRODUCT, XCONFIG_VERSION
 * 2. 编译时定义 DEFAULT_PLATFORM, DEFAULT_PRODUCT, DEFAULT_VERSION
 * 3. 默认使用第一个配置
 *
 * @return 配置指针，失败返回NULL
 */
const xconfig_board_config_t* XCONFIG_SelectDefault(void)
{
    const char *platform = NULL;
    const char *product = NULL;
    const char *version = NULL;
    const xconfig_board_config_t *config = NULL;

    /* 1. 尝试从环境变量读取 */
    platform = getenv("XCONFIG_PLATFORM");
    product = getenv("XCONFIG_PRODUCT");
    version = getenv("XCONFIG_VERSION");

    if (platform != NULL && product != NULL) {
        config = XCONFIG_Find(platform, product, version);
        if (config != NULL) {
            LOG_INFO("XCONFIG", "Selected config from environment: %s/%s/%s",
                     platform, product, version ? version : "any");
            return config;
        }
    }

    /* 2. 尝试使用编译时定义 */
#if defined(DEFAULT_PLATFORM) && defined(DEFAULT_PRODUCT)
    platform = DEFAULT_PLATFORM;
    product = DEFAULT_PRODUCT;
#ifdef DEFAULT_VERSION
    version = DEFAULT_VERSION;
#else
    version = NULL;
#endif

    config = XCONFIG_Find(platform, product, version);
    if (config != NULL) {
        LOG_INFO("XCONFIG", "Selected config from compile-time defaults: %s/%s/%s",
                 platform, product, version ? version : "any");
        return config;
    }
#endif

    /* 3. 使用第一个配置作为默认 */
    if (CONFIG_COUNT > 0) {
        config = g_all_configs[0];
        LOG_INFO("XCONFIG", "Using first config as default: %s/%s/%s",
                 config->platform, config->product, config->version);
        return config;
    }

    LOG_ERROR("XCONFIG", "No configuration available");
    return NULL;
}
