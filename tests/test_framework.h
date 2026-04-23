/************************************************************************
 * 简单的C语言测试框架 - 无外部依赖
 ************************************************************************/

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 测试统计 */
static int test_total __attribute__((unused)) = 0;
static int test_passed __attribute__((unused)) = 0;
static int test_failed __attribute__((unused)) = 0;
static const char *current_test_name __attribute__((unused)) = NULL;

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
        printf("\n========================================\n"); \
        printf("Starting Test Suite\n"); \
        printf("========================================\n\n"); \
    } while(0)

#define TEST_START(name) \
    do { \
        test_total = 0; \
        test_passed = 0; \
        test_failed = 0; \
        printf("\n========================================\n"); \
        printf("Starting %s\n", name); \
        printf("========================================\n\n"); \
    } while(0)

/* 测试结束 */
#define TEST_END() \
    ({ \
        printf("\n========================================\n"); \
        printf("Test Results:\n"); \
        printf("  Total:  %d\n", test_total); \
        printf("  " COLOR_GREEN "Passed: %d" COLOR_RESET "\n", test_passed); \
        if (test_failed > 0) { \
            printf("  " COLOR_RED "Failed: %d" COLOR_RESET "\n", test_failed); \
        } else { \
            printf("  Failed: %d\n", test_failed); \
        } \
        printf("========================================\n"); \
        (test_failed == 0) ? 0 : 1; \
    })

/* 运行测试用例 */
#define RUN_TEST(test_func) \
    do { \
        current_test_name = #test_func; \
        test_total++; \
        printf("[ RUN      ] %s\n", current_test_name); \
        test_func(); \
        printf(COLOR_GREEN "[       OK ] %s\n" COLOR_RESET, current_test_name); \
        test_passed++; \
    } while(0)

/* 断言宏 */
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Assertion failed: %s\n" COLOR_RESET, \
                   __FILE__, __LINE__, #condition); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected not equal to %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(expected)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected NULL pointer\n" COLOR_RESET, \
                   __FILE__, __LINE__); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected non-NULL pointer\n" COLOR_RESET, \
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
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected >= %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(threshold), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected \"%s\", got \"%s\"\n" COLOR_RESET, \
                   __FILE__, __LINE__, (expected), (actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_STRING(expected, actual) TEST_ASSERT_STRING_EQUAL(expected, actual)

#define TEST_ASSERT_LESS_OR_EQUAL(threshold, actual) \
    do { \
        if ((actual) > (threshold)) { \
            printf(COLOR_RED "[  FAILED  ] %s:%d: Expected <= %d, got %d\n" COLOR_RESET, \
                   __FILE__, __LINE__, (int)(threshold), (int)(actual)); \
            test_failed++; \
            return; \
        } \
    } while(0)

/* 测试消息 */
#define TEST_MESSAGE(msg) \
    printf(COLOR_YELLOW "[  INFO    ] %s\n" COLOR_RESET, msg)

/* 忽略测试 */
#define TEST_IGNORE() \
    do { \
        printf(COLOR_YELLOW "[ IGNORED  ] %s\n" COLOR_RESET, current_test_name); \
        test_total--; \
        return; \
    } while(0)

#define TEST_IGNORE_MESSAGE(msg) \
    do { \
        printf(COLOR_YELLOW "[ IGNORED  ] %s: %s\n" COLOR_RESET, current_test_name, msg); \
        test_total--; \
        return; \
    } while(0)

#endif /* TEST_FRAMEWORK_H */
