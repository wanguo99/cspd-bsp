/**
 * @file libtest.h
 * @brief Reusable unit testing framework - Core API
 *
 * This library provides a generic, platform-independent testing framework
 * for C projects. It supports automatic test registration, hierarchical
 * organization, and interactive/CLI execution modes.
 *
 * Dependencies: Only OSAL (for portability)
 */

#ifndef LIBTEST_H
#define LIBTEST_H

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
void libtest_register_suite(const test_suite_t *suite);

/* Core API - Test Execution */
int32_t libtest_run_all(void);
int32_t libtest_run_layer(const str_t *layer_name);
int32_t libtest_run_module(const str_t *module_name);
int32_t libtest_run_suite(const str_t *suite_name);
int32_t libtest_run_test(const str_t *suite_name, const str_t *test_name);

/* Core API - Test Discovery */
void libtest_list_all(void);
void libtest_list_layer(const str_t *layer_name);
void libtest_list_module(const str_t *module_name);

/* Core API - Interactive Mode */
int32_t libtest_interactive_menu(void);

/* Core API - Statistics */
const test_stats_t* libtest_get_stats(void);
void libtest_reset_stats(void);

#endif /* LIBTEST_H */
