/**
 * @file test_registry.h
 * @brief Test registration macros
 *
 * Provides macros for automatic test registration using GCC constructor
 * attributes. Tests self-register at program startup without manual
 * registration in a central file.
 */

#ifndef TEST_REGISTRY_H
#define TEST_REGISTRY_H

#include "libtest.h"

/* Define a test case */
#define TEST_CASE(name) \
    static void name(void)

/* Define a test case with fixtures */
#define TEST_CASE_WITH_FIXTURE(name, setup_func, teardown_func) \
    static void name(void)

/* Begin test suite definition */
#define TEST_SUITE_BEGIN(suite_id, module, layer) \
    static const test_case_t suite_id##_cases[] = {

/* Add test case to suite */
#define TEST_CASE_REF(test_name) \
    { \
        .name = #test_name, \
        .func = test_name, \
        .setup = NULL, \
        .teardown = NULL \
    },

/* End test suite definition and auto-register */
#define TEST_SUITE_END(suite_id, module, layer) \
    }; \
    static const test_suite_t suite_id##_suite = { \
        .suite_name = #suite_id, \
        .module_name = module, \
        .layer_name = layer, \
        .cases = suite_id##_cases, \
        .case_count = sizeof(suite_id##_cases) / sizeof(test_case_t), \
        .suite_setup = NULL, \
        .suite_teardown = NULL \
    }; \
    __attribute__((constructor)) \
    static void register_##suite_id(void) { \
        libtest_register_suite(&suite_id##_suite); \
    }

/* Suite with setup/teardown */
#define TEST_SUITE_END_WITH_FIXTURE(suite_id, module, layer, setup, teardown) \
    }; \
    static const test_suite_t suite_id##_suite = { \
        .suite_name = #suite_id, \
        .module_name = module, \
        .layer_name = layer, \
        .cases = suite_id##_cases, \
        .case_count = sizeof(suite_id##_cases) / sizeof(test_case_t), \
        .suite_setup = setup, \
        .suite_teardown = teardown \
    }; \
    __attribute__((constructor)) \
    static void register_##suite_id(void) { \
        libtest_register_suite(&suite_id##_suite); \
    }

#endif /* TEST_REGISTRY_H */
