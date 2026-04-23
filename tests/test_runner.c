/************************************************************************
 * 测试运行器实现
 ************************************************************************/

#include "test_runner.h"
#include "test_framework.h"

/* 全局测试统计 */
test_stats_t g_test_stats = {0};

/* 测试框架内部变量定义 */
int test_total = 0;
int test_passed = 0;
int test_failed = 0;
const char *current_test_name = NULL;

/* 运行单个测试用例 */
void run_test_case(const char *module_name, const test_case_t *test)
{
    printf("\n[RUN ] %s::%s\n", module_name, test->name);

    /* 设置当前测试名称 */
    current_test_name = test->name;

    /* 重置测试框架统计 */
    int before_failed = test_failed;

    /* 运行测试 */
    test->func();

    /* 检查测试结果 */
    int test_passed_flag = (test_failed == before_failed);

    if (test_passed_flag) {
        printf("[PASS] %s::%s\n", module_name, test->name);
        g_test_stats.passed++;
    } else {
        printf("[FAIL] %s::%s\n", module_name, test->name);

        /* 记录失败的测试用例 */
        if (g_test_stats.failed < MAX_FAILED_TESTS) {
            snprintf(g_test_stats.failed_tests[g_test_stats.failed].module_name,
                     sizeof(g_test_stats.failed_tests[g_test_stats.failed].module_name),
                     "%s", module_name);
            snprintf(g_test_stats.failed_tests[g_test_stats.failed].test_name,
                     sizeof(g_test_stats.failed_tests[g_test_stats.failed].test_name),
                     "%s", test->name);
        }
        g_test_stats.failed++;
    }
    g_test_stats.total++;
}

/* 运行模块的所有测试 */
void run_module_tests(const test_module_t *module)
{
    printf("\n========================================\n");
    printf("Running module: %s\n", module->module_name);
    printf("Total tests: %u\n", module->test_count);
    printf("========================================\n");

    for (uint32_t i = 0; i < module->test_count; i++) {
        run_test_case(module->module_name, &module->test_cases[i]);
    }
}

/* 运行指定模块的指定测试 */
void run_single_test(const test_module_t *module, const char *test_name)
{
    for (uint32_t i = 0; i < module->test_count; i++) {
        if (strcmp(module->test_cases[i].name, test_name) == 0) {
            run_test_case(module->module_name, &module->test_cases[i]);
            return;
        }
    }
    printf("Error: Test '%s' not found in module '%s'\n", test_name, module->module_name);
}

/* 列出模块的所有测试用例 */
void list_module_tests(const test_module_t *module)
{
    printf("\nModule: %s (%u tests)\n", module->module_name, module->test_count);
    for (uint32_t i = 0; i < module->test_count; i++) {
        printf("  [%u] %s\n", i + 1, module->test_cases[i].name);
    }
}

/* 打印测试统计 */
void print_test_stats(void)
{
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total:  %u\n", g_test_stats.total);
    printf("  Passed: %u\n", g_test_stats.passed);
    printf("  Failed: %u\n", g_test_stats.failed);

    if (g_test_stats.failed == 0) {
        printf("\n[SUCCESS] All tests passed!\n");
    } else {
        printf("\n[FAILURE] %u test(s) failed!\n", g_test_stats.failed);
        printf("\nFailed Tests:\n");
        uint32_t count = g_test_stats.failed < MAX_FAILED_TESTS ? g_test_stats.failed : MAX_FAILED_TESTS;
        for (uint32_t i = 0; i < count; i++) {
            printf("  %u. %s::%s\n", i + 1,
                   g_test_stats.failed_tests[i].module_name,
                   g_test_stats.failed_tests[i].test_name);
        }
        if (g_test_stats.failed > MAX_FAILED_TESTS) {
            printf("  ... and %u more\n", g_test_stats.failed - MAX_FAILED_TESTS);
        }
    }
    printf("========================================\n");
}

/* 重置测试统计 */
void reset_test_stats(void)
{
    g_test_stats.total = 0;
    g_test_stats.passed = 0;
    g_test_stats.failed = 0;
}
