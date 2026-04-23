/************************************************************************
 * OSAL单元测试 - 统一入口程序
 * 支持交互式菜单和命令行参数
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* 外部测试模块声明 */
extern const test_module_t test_os_file;
extern const test_module_t test_os_network;
extern const test_module_t test_os_signal;
extern const test_module_t test_os_task;
extern const test_module_t test_os_mutex;
extern const test_module_t test_os_queue;

/* 所有测试模块列表 */
static const test_module_t *all_modules[] = {
    &test_os_file,
    &test_os_network,
    &test_os_signal,
    &test_os_task,
    &test_os_mutex,
    &test_os_queue,
};

#define MODULE_COUNT (sizeof(all_modules) / sizeof(test_module_t *))

/* 打印使用说明 */
void print_usage(const char *prog_name)
{
    printf("Usage: %s [options]\n", prog_name);
    printf("\nOptions:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -l, --list              List all modules and tests\n");
    printf("  -a, --all               Run all tests in all modules\n");
    printf("  -m <module>             Run all tests in specified module\n");
    printf("  -t <module> <test>      Run specific test in module\n");
    printf("  -i, --interactive       Interactive mode (default)\n");
    printf("\nExamples:\n");
    printf("  %s -a                   # Run all tests\n", prog_name);
    printf("  %s -m test_os_file      # Run all file I/O tests\n", prog_name);
    printf("  %s -t test_os_file test_OS_FileOpen_Close_Success\n", prog_name);
    printf("  %s -i                   # Interactive menu\n", prog_name);
}

/* 列出所有模块和测试 */
void list_all_modules(void)
{
    printf("\n========================================\n");
    printf("Available Test Modules:\n");
    printf("========================================\n");

    for (uint32_t i = 0; i < MODULE_COUNT; i++) {
        list_module_tests(all_modules[i]);
    }
}

/* 运行所有模块的所有测试 */
void run_all_tests(void)
{
    reset_test_stats();

    printf("\n========================================\n");
    printf("Running ALL Tests\n");
    printf("========================================\n");

    for (uint32 i = 0; i < MODULE_COUNT; i++) {
        run_module_tests(all_modules[i]);
    }

    print_test_stats();
}

/* 根据名称查找模块 */
const test_module_t *find_module(const char *module_name)
{
    for (uint32 i = 0; i < MODULE_COUNT; i++) {
        if (strcmp(all_modules[i]->module_name, module_name) == 0) {
            return all_modules[i];
        }
    }
    return NULL;
}

/* 交互式菜单 */
void interactive_menu(void)
{
    char input[256];
    int choice;

    while (1) {
        printf("\n========================================\n");
        printf("OSAL Unit Test - Interactive Menu\n");
        printf("========================================\n");
        printf("1. Run all tests\n");
        printf("2. List all modules\n");
        printf("3. Run tests by module\n");
        printf("4. Run single test\n");
        printf("0. Exit\n");
        printf("========================================\n");
        printf("Enter your choice: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        choice = atoi(input);

        switch (choice) {
            case 0:
                printf("Exiting...\n");
                return;

            case 1:
                run_all_tests();
                break;

            case 2:
                list_all_modules();
                break;

            case 3: {
                printf("\nAvailable modules:\n");
                for (uint32 i = 0; i < MODULE_COUNT; i++) {
                    printf("  [%u] %s\n", i + 1, all_modules[i]->module_name);
                }
                printf("Enter module number or name: ");

                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }

                /* 去除换行符 */
                input[strcspn(input, "\n")] = 0;

                /* 尝试作为数字解析 */
                int module_idx = atoi(input);
                const test_module_t *module = NULL;

                if (module_idx > 0 && module_idx <= (int)MODULE_COUNT) {
                    module = all_modules[module_idx - 1];
                } else {
                    module = find_module(input);
                }

                if (module) {
                    reset_test_stats();
                    run_module_tests(module);
                    print_test_stats();
                } else {
                    printf("Error: Module not found\n");
                }
                break;
            }

            case 4: {
                printf("\nAvailable modules:\n");
                for (uint32 i = 0; i < MODULE_COUNT; i++) {
                    printf("  [%u] %s\n", i + 1, all_modules[i]->module_name);
                }
                printf("Enter module number or name: ");

                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                input[strcspn(input, "\n")] = 0;

                int module_idx = atoi(input);
                const test_module_t *module = NULL;

                if (module_idx > 0 && module_idx <= (int)MODULE_COUNT) {
                    module = all_modules[module_idx - 1];
                } else {
                    module = find_module(input);
                }

                if (!module) {
                    printf("Error: Module not found\n");
                    break;
                }

                list_module_tests(module);
                printf("Enter test number or name: ");

                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                input[strcspn(input, "\n")] = 0;

                int test_idx = atoi(input);
                const char *test_name = NULL;

                if (test_idx > 0 && test_idx <= (int)module->test_count) {
                    test_name = module->test_cases[test_idx - 1].name;
                } else {
                    test_name = input;
                }

                reset_test_stats();
                run_single_test(module, test_name);
                print_test_stats();
                break;
            }

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}

/* 主函数 */
int main(int argc, char *argv[])
{
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

        if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
            list_all_modules();
            return 0;
        }

        if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0) {
            run_all_tests();
            return (g_test_stats.failed == 0) ? 0 : 1;
        }

        if (strcmp(argv[1], "-m") == 0 && argc >= 3) {
            const test_module_t *module = find_module(argv[2]);
            if (!module) {
                printf("Error: Module '%s' not found\n", argv[2]);
                return 1;
            }
            reset_test_stats();
            run_module_tests(module);
            print_test_stats();
            return (g_test_stats.failed == 0) ? 0 : 1;
        }

        if (strcmp(argv[1], "-t") == 0 && argc >= 4) {
            const test_module_t *module = find_module(argv[2]);
            if (!module) {
                printf("Error: Module '%s' not found\n", argv[2]);
                return 1;
            }
            reset_test_stats();
            run_single_test(module, argv[3]);
            print_test_stats();
            return (g_test_stats.failed == 0) ? 0 : 1;
        }
    }

    /* 无效参数 */
    printf("Error: Invalid arguments\n\n");
    print_usage(argv[0]);
    return 1;
}
