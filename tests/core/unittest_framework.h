/************************************************************************
 * 简单的C语言测试框架 - 无外部依赖
 ************************************************************************/

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "osapi_log.h"

/* 测试统计 - 声明为extern，在unittest_runner.c中定义 */
extern int test_total;
extern int test_passed;
extern int test_failed;
extern const char *current_test_name;

/* 颜色定义 */
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RESET   "\033[0m"

/* 测试开始 */
#define TEST_BEGIN() \
    do { \
        test_total = 0; \
        test_passed = 0; \
        test_failed = 0; \
        OS_printf("\n========================================\n"); \
        OS_printf("Starting Test Suite\n"); \
        OS_printf("========================================\n\n"); \
    } while(0)

#define TEST_START(name) \
    do { \
        test_total = 0; \
        test_passed = 0; \
        test_failed = 0; \
        OS_printf("\n========================================\n"); \
        OS_printf("Starting %s\n", name); \
        OS_printf("========================================\n\n"); \
    } while(0)

/* 测试结束 */
#define TEST_END() \
    ({ \
        OS_printf("\n========================================\n"); \
        OS_printf("Test Results:\n"); \
        OS_printf("  Total:   %d\n", test_total); \
        OS_printf("  " COLOR_GREEN "Passed:  %d" COLOR_RESET "\n", test_passed); \
        if (test_failed > 0) { \
            OS_printf("  " COLOR_RED "Failed:  %d" COLOR_RESET "\n", test_failed); \
        } else { \
            OS_printf("  Failed:  %d\n", test_failed); \
        } \
        OS_printf("========================================\n"); \
        (test_failed == 0) ? 0 : 1; \
    })

/* 运行测试用例 */
#define RUN_TEST(test_func) \
    do { \
        current_test_name = #test_func; \
        test_total++; \
        OS_printf("[ RUN      ] %s\n", current_test_name ? current_test_name : "Unknown"); \
        test_func(); \
        OS_printf(COLOR_GREEN "[       OK ] %s\n" COLOR_RESET, current_test_name ? current_test_name : "Unknown"); \
        test_passed++; \
    } while(0)

/* 断言宏 */
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Assertion failed: %s\n" COLOR_RESET, \
                   __FILE__, __LINE__, #condition); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected not equal to %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(expected)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected NULL pointer\n" COLOR_RESET, \
                   __FILE__, __LINE__); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected non-NULL pointer\n" COLOR_RESET, \
                   __FILE__, __LINE__); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))

#define TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual) \
    do { \
        if ((actual) < (threshold)) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected >= %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(threshold), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected \"%s\", got \"%s\"\n" COLOR_RESET, \
                   __FILE__, __LINE__, (expected), (actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) TEST_ASSERT_STRING_EQUAL(expected, actual)

#define TEST_ASSERT_LESS_OR_EQUAL(threshold, actual) \
    do { \
        if ((actual) > (threshold)) { \
            OS_printf(COLOR_RED "[  FAILED  ] %s:%d: Expected <= %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(threshold), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

/* 测试消息 */
#define TEST_MESSAGE(msg) \
    OS_printf(COLOR_YELLOW "[  INFO    ] %s\n" COLOR_RESET, msg)

#define TEST_WARNING(msg) \
    OS_printf(COLOR_YELLOW "[ WARNING  ] %s\n" COLOR_RESET, msg)

#endif /* TEST_FRAMEWORK_H */
