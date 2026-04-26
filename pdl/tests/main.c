/**
 * @file main.c
 * @brief PDL模块独立测试入口
 *
 * 本文件提供PDL模块的独立测试程序入口，支持：
 * - 独立编译：可单独编译PDL测试程序
 * - 完整功能：支持交互式菜单、命令行参数
 * - 依赖HAL/OSAL：PDL层依赖HAL和OSAL
 */

#include "libutest.h"
#include "osal.h"
#include <stdlib.h>

/* 打印使用说明 */
static void print_usage(const str_t *prog_name)
{
    OSAL_Printf("Usage: %s [options]\n", prog_name);
    OSAL_Printf("\n");
    OSAL_Printf("Options:\n");
    OSAL_Printf("  -h, --help              Show this help message\n");
    OSAL_Printf("  -i, --interactive       Interactive menu mode (default)\n");
    OSAL_Printf("  -a, --all               Run all PDL tests\n");
    OSAL_Printf("  -l, --list              List all PDL tests\n");
    OSAL_Printf("  -m, --module <name>     Run specific module tests\n");
    OSAL_Printf("  -s, --suite <name>      Run specific test suite\n");
    OSAL_Printf("  -t, --test <suite> <test>  Run specific test case\n");
    OSAL_Printf("\n");
    OSAL_Printf("Examples:\n");
    OSAL_Printf("  %s                      # Interactive menu\n", prog_name);
    OSAL_Printf("  %s -a                   # Run all tests\n", prog_name);
    OSAL_Printf("  %s -l                   # List all tests\n", prog_name);
    OSAL_Printf("  %s -m test_satellite    # Run satellite module tests\n", prog_name);
    OSAL_Printf("  %s -s test_satellite_suite  # Run specific suite\n", prog_name);
    OSAL_Printf("\n");
}

/* 主函数 */
int32_t main(int32_t argc, str_t *argv[])
{
    int32_t ret = 0;

    OSAL_Printf("\n");
    OSAL_Printf("========================================\n");
    OSAL_Printf("  PDL Module Unit Test\n");
    OSAL_Printf("  Version: %s\n", OS_GetVersionString());
    OSAL_Printf("========================================\n");
    OSAL_Printf("\n");

    /* 无参数或-i：交互式菜单 */
    if (argc == 1 || (argc == 2 && (OSAL_Strcmp(argv[1], "-i") == 0 ||
                                     OSAL_Strcmp(argv[1], "--interactive") == 0))) {
        ret = libutest_interactive_menu();
    }
    /* -h: 帮助 */
    else if (argc == 2 && (OSAL_Strcmp(argv[1], "-h") == 0 ||
                           OSAL_Strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        ret = 0;
    }
    /* -a: 运行所有测试 */
    else if (argc == 2 && (OSAL_Strcmp(argv[1], "-a") == 0 ||
                           OSAL_Strcmp(argv[1], "--all") == 0)) {
        ret = libutest_run_layer("PDL");
    }
    /* -l: 列出所有测试 */
    else if (argc == 2 && (OSAL_Strcmp(argv[1], "-l") == 0 ||
                           OSAL_Strcmp(argv[1], "--list") == 0)) {
        libutest_list_layer("PDL");
        ret = 0;
    }
    /* -m: 运行指定模块 */
    else if (argc == 3 && (OSAL_Strcmp(argv[1], "-m") == 0 ||
                           OSAL_Strcmp(argv[1], "--module") == 0)) {
        ret = libutest_run_module(argv[2]);
    }
    /* -s: 运行指定测试套件 */
    else if (argc == 3 && (OSAL_Strcmp(argv[1], "-s") == 0 ||
                           OSAL_Strcmp(argv[1], "--suite") == 0)) {
        ret = libutest_run_suite(argv[2]);
    }
    /* -t: 运行指定测试用例 */
    else if (argc == 4 && (OSAL_Strcmp(argv[1], "-t") == 0 ||
                           OSAL_Strcmp(argv[1], "--test") == 0)) {
        ret = libutest_run_test(argv[2], argv[3]);
    }
    /* 未知参数 */
    else {
        OSAL_Printf("Error: Unknown option or invalid arguments\n\n");
        print_usage(argv[0]);
        ret = 1;
    }

    /* 打印测试统计 */
    if (ret != 0 && argc > 1 && OSAL_Strcmp(argv[1], "-h") != 0 &&
        OSAL_Strcmp(argv[1], "--help") != 0 &&
        OSAL_Strcmp(argv[1], "-l") != 0 &&
        OSAL_Strcmp(argv[1], "--list") != 0) {
        const test_stats_t *stats = libutest_get_stats();
        OSAL_Printf("\n");
        OSAL_Printf("========================================\n");
        OSAL_Printf("  Test Summary\n");
        OSAL_Printf("========================================\n");
        OSAL_Printf("  Total:   %u\n", stats->total);
        OSAL_Printf("  Passed:  %u\n", stats->passed);
        OSAL_Printf("  Failed:  %u\n", stats->failed);
        OSAL_Printf("  Skipped: %u\n", stats->skipped);
        OSAL_Printf("========================================\n");
        OSAL_Printf("\n");
    }

    return ret;
}
