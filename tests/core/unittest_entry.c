/************************************************************************
 * CSPD-BSP 统一测试入口程序
 * 支持交互式菜单和命令行参数
 * 整合所有层的测试：OSAL、HAL、Service、Apps
 ************************************************************************/

#include "unittest_runner.h"
#include "unittest_framework.h"
#include "osal_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

/* 设置终端为非缓冲模式 */
void set_terminal_mode(int enable)
{
    static struct termios old_term, new_term;

    if (enable) {
        tcgetattr(STDIN_FILENO, &old_term);
        new_term = old_term;
        new_term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    }
}

/* 等待用户按键 */
void wait_for_key(void)
{
    OSAL_Printf("\nPress 'q' to quit, or any other key to continue...");
    fflush(stdout);

    set_terminal_mode(1);
    int ch = getchar();
    set_terminal_mode(0);

    OSAL_Printf("\n");

    if (ch == 'q' || ch == 'Q') {
        OSAL_Printf("Exiting...\n");
        exit(0);
    }
}

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
    LOG_INFO("TEST", "Usage: %s [options]", prog_name);
    LOG_INFO("TEST", "\nOptions:");
    LOG_INFO("TEST", "  -h, --help              Show this help message");
    LOG_INFO("TEST", "  -l, --list              List all layers, modules and tests");
    LOG_INFO("TEST", "  -a, --all               Run all tests in all layers");
    LOG_INFO("TEST", "  -L <layer>              Run all tests in specified layer (OSAL/HAL/Service/Apps)");
    LOG_INFO("TEST", "  -m <module>             Run all tests in specified module");
    LOG_INFO("TEST", "  -t <module> <test>      Run specific test in module");
    LOG_INFO("TEST", "  -i, --interactive       Interactive mode (default)");
    LOG_INFO("TEST", "\nExamples:");
    LOG_INFO("TEST", "  %s -a                   # Run all tests", prog_name);
    LOG_INFO("TEST", "  %s -L OSAL              # Run all OSAL layer tests", prog_name);
    LOG_INFO("TEST", "  %s -m test_os_file      # Run all file I/O tests", prog_name);
    LOG_INFO("TEST", "  %s -t test_os_file test_OS_FileOpen_Close_Success", prog_name);
    LOG_INFO("TEST", "  %s -i                   # Interactive menu", prog_name);
}

/* 列出所有层、模块和测试 */
void list_all_tests(void)
{
    LOG_INFO("TEST", "\n========================================");
    LOG_INFO("TEST", "CSPD-BSP Test Suite");
    LOG_INFO("TEST", "========================================");

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        LOG_INFO("TEST", "\n[Layer] %s (%u modules)",
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

    LOG_INFO("TEST", "\n========================================");
    LOG_INFO("TEST", "Running ALL Tests");
    LOG_INFO("TEST", "========================================");

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        LOG_INFO("TEST", "\n>>> Testing Layer: %s", all_layers[i].layer_name);

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
            LOG_INFO("TEST", "\n========================================");
            LOG_INFO("TEST", "Running %s Layer Tests", all_layers[i].layer_name);
            LOG_INFO("TEST", "========================================");

            for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
                run_module_tests(all_layers[i].modules[j]);
            }

            print_test_stats();
            return;
        }
    }

    LOG_ERROR("TEST", "Error: Layer '%s' not found", layer_name);
    LOG_INFO("TEST", "Available layers: OSAL, HAL, Service, Apps");
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
        LOG_ERROR("TEST", "Error: Module '%s' not found", module_name);
        LOG_INFO("TEST", "Use -l to list all available modules");
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
        LOG_ERROR("TEST", "Error: Module '%s' not found", module_name);
        LOG_INFO("TEST", "Use -l to list all available modules");
    }
}

/* 交互式运行单个测试（三级选择：层级->模块->测试） */
void interactive_run_single_test(void)
{
    char input[256];

    /* 第一步：显示所有层级 */
    OSAL_Printf("\nAvailable layers:\n");
    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        OSAL_Printf("  [%u] %s (%u modules)\n", i + 1, all_layers[i].layer_name, all_layers[i].module_count);
    }

    OSAL_Printf("\nEnter layer number: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return;
    }

    uint32_t layer_num = (uint32_t)atoi(input);
    if (layer_num == 0 || layer_num > LAYER_COUNT) {
        OSAL_Printf("Invalid layer number\n");
        return;
    }

    const test_layer_t *selected_layer = &all_layers[layer_num - 1];

    /* 第二步：显示该层级的所有模块 */
    OSAL_Printf("\nAvailable modules in %s layer:\n", selected_layer->layer_name);
    for (uint32_t i = 0; i < selected_layer->module_count; i++) {
        OSAL_Printf("  [%u] %s\n", i + 1, selected_layer->modules[i]->module_name);
    }

    OSAL_Printf("\nEnter module number: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return;
    }

    uint32_t module_num = (uint32_t)atoi(input);
    if (module_num == 0 || module_num > selected_layer->module_count) {
        OSAL_Printf("Invalid module number\n");
        return;
    }

    const test_module_t *selected_module = selected_layer->modules[module_num - 1];

    /* 第三步：显示该模块的所有测试用例 */
    OSAL_Printf("\nAvailable tests in %s:\n", selected_module->module_name);
    for (uint32_t i = 0; i < selected_module->test_count; i++) {
        OSAL_Printf("  [%u] %s\n", i + 1, selected_module->test_cases[i].name);
    }

    OSAL_Printf("\nEnter test number: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return;
    }

    uint32_t test_num = (uint32_t)atoi(input);
    if (test_num == 0 || test_num > selected_module->test_count) {
        OSAL_Printf("Invalid test number\n");
        return;
    }

    /* 运行选中的测试 */
    reset_test_stats();
    run_test_case(selected_module->module_name, &selected_module->test_cases[test_num - 1]);
    print_test_stats();
}

/* 交互式运行指定模块的所有测试 */
void interactive_run_module_tests(void)
{
    char input[256];

    OSAL_Printf("\nAvailable modules:\n");
    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        OSAL_Printf("  [%s Layer]\n", all_layers[i].layer_name);
        for (uint32_t j = 0; j < all_layers[i].module_count; j++) {
            OSAL_Printf("    - %s\n", all_layers[i].modules[j]->module_name);
        }
    }

    OSAL_Printf("\nEnter module name: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return;
    }

    input[strcspn(input, "\n")] = 0;  // 移除换行符
    run_module_by_name(input);
}

/* 交互式菜单 */
void interactive_menu(void)
{
    char input[256];
    int choice;

    while (1) {
        OSAL_Printf("\n========================================\n");
        OSAL_Printf("CSPD-BSP Test Suite - Interactive Menu\n");
        OSAL_Printf("========================================\n");
        OSAL_Printf("1. Run all tests (all layers)\n");
        OSAL_Printf("2. Run OSAL layer tests\n");
        OSAL_Printf("3. Run HAL layer tests\n");
        OSAL_Printf("4. Run Service layer tests\n");
        OSAL_Printf("5. Run Apps layer tests\n");
        OSAL_Printf("6. Run specific module tests\n");
        OSAL_Printf("7. Run single test\n");
        OSAL_Printf("8. List all tests\n");
        OSAL_Printf("0. Exit\n");
        OSAL_Printf("========================================\n");
        OSAL_Printf("Enter your choice (or 'q' to quit): ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        /* 检查是否输入q退出 */
        if (input[0] == 'q' || input[0] == 'Q') {
            OSAL_Printf("\nExiting...\n");
            return;
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

            case 6:
                interactive_run_module_tests();
                break;

            case 7:
                interactive_run_single_test();
                break;

            case 8:
                list_all_tests();
                break;

            case 0:
                OSAL_Printf("\nExiting...\n");
                return;

            default:
                OSAL_Printf("\nInvalid choice. Please try again.\n");
                break;
        }

        wait_for_key();
    }
}

/* 主函数 */
int main(int argc, char *argv[])
{
    OSAL_Printf("\n========================================\n");
    OSAL_Printf("CSPD-BSP Unified Test Suite\n");
    OSAL_Printf("Version 1.0.0\n");
    OSAL_Printf("========================================\n");

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
            OSAL_Printf("Error: Unknown option '%s'\n", argv[1]);
            print_usage(argv[0]);
            return 1;
        }
    }

    return 0;
}
