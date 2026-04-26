/**
 * @file libutest.h
 * @brief libutest - 通用单元测试框架
 *
 * 提供通用的、平台无关的测试框架，支持：
 * - 自动测试注册
 * - 层级化组织
 * - 交互式菜单和命令行模式
 *
 * 依赖：仅依赖OSAL（保证可移植性）
 */

#ifndef LIBUTEST_H
#define LIBUTEST_H

#include "osal_types.h"

/* Test function signatures */
typedef void (*test_func_t)(void);
typedef void (*fixture_func_t)(void);

/* Test case structure */
typedef struct {
    const str_t *name;
    test_func_t func;
    fixture_func_t setup;
    fixture_func_t teardown;
} test_case_t;

/* Test suite structure */
typedef struct {
    const str_t *suite_name;
    const str_t *module_name;
    const str_t *layer_name;
    const test_case_t *cases;
    uint32_t case_count;
    fixture_func_t suite_setup;
    fixture_func_t suite_teardown;
} test_suite_t;

/* Test statistics */
typedef struct {
    uint32_t total;
    uint32_t passed;
    uint32_t failed;
    uint32_t skipped;
} test_stats_t;

/* Test result codes */
typedef enum {
    TEST_RESULT_PASS = 0,
    TEST_RESULT_FAIL = 1,
    TEST_RESULT_SKIP = 2
} test_result_t;

/* Core API - Test Registration */
void libutest_register_suite(const test_suite_t *suite);

/* Core API - Test Execution */
int32_t libutest_run_all(void);
int32_t libutest_run_layer(const str_t *layer_name);
int32_t libutest_run_module(const str_t *module_name);
int32_t libutest_run_suite(const str_t *suite_name);
int32_t libutest_run_test(const str_t *suite_name, const str_t *test_name);

/* Core API - Test Discovery */
void libutest_list_all(void);
void libutest_list_layer(const str_t *layer_name);
void libutest_list_module(const str_t *module_name);

/* Core API - Interactive Mode */
int32_t libutest_interactive_menu(void);

/* Core API - Statistics */
const test_stats_t* libutest_get_stats(void);
void libutest_reset_stats(void);

#endif /* LIBUTEST_H */
