/************************************************************************
 * 测试运行器框架 - 支持模块化和交互式测试
 ************************************************************************/

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* 使用标准C类型 */
typedef uint32_t uint32;
#include <stdlib.h>

/* 测试用例函数指针类型 */
typedef void (*test_func_t)(void);

/* 单个测试用例结构 */
typedef struct {
    const char *name;        /* 测试用例名称 */
    test_func_t func;        /* 测试函数指针 */
} test_case_t;

/* 测试模块结构 */
typedef struct {
    const char *module_name;      /* 模块名称 */
    const test_case_t *test_cases; /* 测试用例数组 */
    uint32 test_count;            /* 测试用例数量 */
} test_module_t;

/* 失败测试用例记录 */
#define MAX_FAILED_TESTS 256
typedef struct {
    char module_name[64];
    char test_name[128];
} failed_test_t;

/* 测试统计 */
typedef struct {
    uint32 total;
    uint32 passed;
    uint32 failed;
    failed_test_t failed_tests[MAX_FAILED_TESTS];
} test_stats_t;

/* 全局测试统计 */
extern test_stats_t g_test_stats;

/* 测试模块注册宏 - 仅在非独立测试模式下使用 */
#ifndef STANDALONE_TEST

#define TEST_MODULE_BEGIN(module_name) \
    static const test_case_t module_name##_cases[] = {

#define TEST_CASE(func) \
    { #func, func },

#define TEST_MODULE_END(module_name) \
    }; \
    const test_module_t module_name = { \
        #module_name, \
        module_name##_cases, \
        sizeof(module_name##_cases) / sizeof(test_case_t) \
    };

#else
/* 在独立测试模式下，这些宏不生成任何代码 */
#define TEST_MODULE_BEGIN(module_name)
#define TEST_CASE(func)
#define TEST_MODULE_END(module_name)

#endif

/* 运行单个测试用例 */
void run_test_case(const char *module_name, const test_case_t *test);

/* 运行模块的所有测试 */
void run_module_tests(const test_module_t *module);

/* 运行指定模块的指定测试 */
void run_single_test(const test_module_t *module, const char *test_name);

/* 列出模块的所有测试用例 */
void list_module_tests(const test_module_t *module);

/* 打印测试统计 */
void print_test_stats(void);

/* 重置测试统计 */
void reset_test_stats(void);

#endif /* TEST_RUNNER_H */
