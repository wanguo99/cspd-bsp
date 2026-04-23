/************************************************************************
 * CSPD-BSP 统一测试入口程序
 * 支持交互式菜单和命令行参数
 * 整合所有层的测试：OSAL、HAL、Service、Apps
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 外部测试模块声明 - OSAL层 */
extern const test_module_t test_os_file;
extern const test_module_t test_os_network;
extern const test_module_t test_os_signal;
extern const test_module_t test_os_task;
extern const test_module_t test_os_mutex;
extern const test_module_t test_os_queue;

/* 外部测试模块声明 - HAL层 */
extern const test_module_t test_hal_can;

/* 外部测试模块声明 - Service层 */
extern const test_module_t test_payload_service;

/* 外部测试模块声明 - Apps层 */
extern const test_module_t test_can_gateway;
extern const test_module_t test_protocol_converter;

/* 测试层级分类 */
typedef struct {
    const char *layer_name;
    const test_module_t **modules;
    uint32_t module_count;
} test_layer_t;

/* OSAL层测试模块 */
static const test_module_t *osal_modules[] = {
    &test_os_file,
    &test_os_network,
    &test_os_signal,
    &test_os_task,
    &test_os_mutex,
    &test_os_queue,
};

/* HAL层测试模块 */
static const test_module_t *hal_modules[] = {
    &test_hal_can,
};

/* Service层测试模块 */
static const test_module_t *service_modules[] = {
    &test_payload_service,
};

/* Apps层测试模块 */
static const test_module_t *apps_modules[] = {
    &test_can_gateway,
    &test_protocol_converter,
};

/* 所有测试层 */
static test_layer_t all_layers[] = {
    {"OSAL", osal_modules, sizeof(osal_modules) / sizeof(test_module_t *)},
    {"HAL", hal_modules, sizeof(hal_modules) / sizeof(test_module_t *)},
    {"Service", service_modules, sizeof(service_modules) / sizeof(test_module_t *)},
    {"Apps", apps_modules, sizeof(apps_modules) / sizeof(test_module_t *)},
};

#define LAYER_COUNT (sizeof(all_layers) / sizeof(test_layer_t))

/* 打印使用说明 */
void print_usage(const char *prog_name)
{
    printf("Usage: %s [options]\n", prog_name);
    printf("\nOptions:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -l, --list              List all layers, modules and tests\n");
    printf("  -a, --all               Run all tests in all layers\n");
    printf("  -L <layer>              Run all tests in specified layer (OSAL/HAL/Service/Apps)\n");
    printf("  -m <module>             Run all tests in specified module\n");
    printf("  -t <module> <test>      Run specific test in module\n");
    printf("  -i, --interactive       Interactive mode (default)\n");
    printf("\nExamples:\n");
    printf("  %s -a                   # Run all tests\n", prog_name);
    printf("  %s -L OSAL              # Run all OSAL layer tests\n", prog_name);
    printf("  %s -m test_os_file      # Run all file I/O tests\n", prog_name);
    printf("  %s -t test_os_file test_OS_FileOpen_Close_Success\n", prog_name);
    printf("  %s -i                   # Interactive menu\n", prog_name);
}

/* 列出所有层、模块和测试 */
void list_all_tests(void)
{
    printf("\n========================================\n");
    printf("CSPD-BSP Test Suite\n");
    printf("========================================\n");

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        printf("\n[Layer] %s (%u modules)\n",
               all_layers[i].layer_name,
               all_layers[i].module_count);

        for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
            list_module_tests(all_layers[i].modules[j]);
        }
    }
}

/* 运行所有测试 */
void run_all_tests(void)
{
    reset_test_stats();

    printf("\n========================================\n");
    printf("Running ALL Tests\n");
    printf("========================================\n");

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        printf("\n>>> Testing Layer: %s\n", all_layers[i].layer_name);

        for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
            run_module_tests(all_layers[i].modules[j]);
        }
    }

    print_test_stats();
}

/* 运行指定层的所有测试 */
void run_layer_tests(const char *layer_name)
{
    reset_test_stats();

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        if (strcasecmp(all_layers[i].layer_name, layer_name) == 0) {
            printf("\n========================================\n");
            printf("Running %s Layer Tests\n", all_layers[i].layer_name);
            printf("========================================\n");

            for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
                run_module_tests(all_layers[i].modules[j]);
            }

            print_test_stats();
            return;
        }
    }

    printf("Error: Layer '%s' not found\n", layer_name);
    printf("Available layers: OSAL, HAL, Service, Apps\n");
}

/* 根据名称查找模块 */
const test_module_t *find_module(const char *module_name)
{
    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
            if (strcmp(all_layers[i].modules[j]->module_name, module_name) == 0) {
                return all_layers[i].modules[j];
            }
        }
    }
    return NULL;
}

/* 运行指定模块的所有测试 */
void run_module_by_name(const char *module_name)
{
    const test_module_t *module = find_module(module_name);

    if (module) {
        reset_test_stats();
        run_module_tests(module);
        print_test_stats();
    } else {
        printf("Error: Module '%s' not found\n", module_name);
        printf("Use -l to list all available modules\n");
    }
}

/* 运行指定模块的指定测试 */
void run_single_test_by_name(const char *module_name, const char *test_name)
{
    const test_module_t *module = find_module(module_name);

    if (module) {
        reset_test_stats();
        run_single_test(module, test_name);
        print_test_stats();
    } else {
        printf("Error: Module '%s' not found\n", module_name);
        printf("Use -l to list all available modules\n");
    }
}

/* 交互式菜单 */
void interactive_menu(void)
{
    char input[256];
    int choice;

    while (1) {
        printf("\n========================================\n");
        printf("CSPD-BSP Test Suite - Interactive Menu\n");
        printf("========================================\n");
        printf("1. Run all tests (all layers)\n");
        printf("2. Run OSAL layer tests\n");
        printf("3. Run HAL layer tests\n");
        printf("4. Run Service layer tests\n");
        printf("5. Run Apps layer tests\n");
        printf("6. Run specific module tests\n");
        printf("7. Run single test\n");
        printf("8. List all tests\n");
        printf("0. Exit\n");
        printf("========================================\n");
        printf("Enter your choice: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        choice = atoi(input);

        switch (choice) {
            case 1:
                run_all_tests();
                break;

            case 2:
                run_layer_tests("OSAL");
                break;

            case 3:
                run_layer_tests("HAL");
                break;

            case 4:
                run_layer_tests("Service");
                break;

            case 5:
                run_layer_tests("Apps");
                break;

            case 6: {
                printf("\nAvailable modules:\n");
                for (uint32_t i = 0; i < LAYER_COUNT; i++) {
                    printf("  [%s Layer]\n", all_layers[i].layer_name);
                    for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
                        printf("    - %s\n", all_layers[i].modules[j]->module_name);
                    }
                }
                printf("\nEnter module name: ");
                if (fgets(input, sizeof(input), stdin)) {
                    input[strcspn(input, "\n")] = 0;  // 移除换行符
                    run_module_by_name(input);
                }
                break;
            }

            case 7: {
                char module_name[128], test_name[128];
                printf("\nEnter module name: ");
                if (fgets(module_name, sizeof(module_name), stdin)) {
                    module_name[strcspn(module_name, "\n")] = 0;

                    const test_module_t *module = find_module(module_name);
                    if (module) {
                        list_module_tests(module);
                        printf("\nEnter test name: ");
                        if (fgets(test_name, sizeof(test_name), stdin)) {
                            test_name[strcspn(test_name, "\n")] = 0;
                            run_single_test_by_name(module_name, test_name);
                        }
                    } else {
                        printf("Module '%s' not found\n", module_name);
                    }
                }
                break;
            }

            case 8:
                list_all_tests();
                break;

            case 0:
                printf("\nExiting...\n");
                return;

            default:
                printf("\nInvalid choice. Please try again.\n");
                break;
        }

        printf("\nPress Enter to continue...");
        fgets(input, sizeof(input), stdin);
    }
}

/* 主函数 */
int main(int argc, char *argv[])
{
    printf("\n========================================\n");
    printf("CSPD-BSP Unified Test Suite\n");
    printf("Version 1.0.0\n");
    printf("========================================\n");

    /* 无参数或-i参数：交互模式 */
    if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0))) {
        interactive_menu();
        return 0;
    }

    /* 解析命令行参数 */
    if (argc >= 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
            list_all_tests();
            return 0;
        }
        else if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0) {
            run_all_tests();
            return (g_test_stats.failed == 0) ? 0 : 1;
        }
        else if (strcmp(argv[1], "-L") == 0 && argc >= 3) {
            run_layer_tests(argv[2]);
            return (g_test_stats.failed == 0) ? 0 : 1;
        }
        else if (strcmp(argv[1], "-m") == 0 && argc >= 3) {
            run_module_by_name(argv[2]);
            return (g_test_stats.failed == 0) ? 0 : 1;
        }
        else if (strcmp(argv[1], "-t") == 0 && argc >= 4) {
            run_single_test_by_name(argv[2], argv[3]);
            return (g_test_stats.failed == 0) ? 0 : 1;
        }
        else {
            printf("Error: Unknown option '%s'\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }

    return 0;
}
