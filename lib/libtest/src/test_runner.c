/**
 * @file test_runner.c
 * @brief Test execution engine
 *
 * Executes test cases and suites, tracks statistics, and reports results.
 */

#include "libtest.h"
#include "test_assert.h"
#include "osal.h"

#define MAX_SUITES 128

/* Global state for assertions */
bool g_test_failed = false;
const str_t *g_current_test = NULL;

/* Global statistics */
static test_stats_t g_stats = {0};

/* External registry functions */
extern const test_suite_t** libtest_get_all_suites(uint32_t *count);
extern const test_suite_t* libtest_find_suite(const str_t *name);
extern uint32_t libtest_get_suites_by_layer(const str_t *layer_name, const test_suite_t **suites, uint32_t max_suites);
extern uint32_t libtest_get_suites_by_module(const str_t *module_name, const test_suite_t **suites, uint32_t max_suites);

/**
 * Run a single test case
 */
static test_result_t run_test_case(const test_case_t *test)
{
    if (test == NULL || test->func == NULL) {
        return TEST_RESULT_FAIL;
    }

    g_test_failed = false;
    g_current_test = test->name;

    OSAL_Printf("[ RUN      ] %s\n", test->name);

    /* Run setup if provided */
    if (test->setup != NULL) {
        test->setup();
    }

    /* Run test */
    test->func();

    /* Run teardown if provided */
    if (test->teardown != NULL) {
        test->teardown();
    }

    /* Check result */
    if (g_test_failed) {
        OSAL_Printf("[  FAILED  ] %s\n", test->name);
        return TEST_RESULT_FAIL;
    } else {
        OSAL_Printf("[       OK ] %s\n", test->name);
        return TEST_RESULT_PASS;
    }
}

/**
 * Run all tests in a suite
 */
static int32_t run_suite(const test_suite_t *suite)
{
    if (suite == NULL) {
        return OS_ERROR;
    }

    OSAL_Printf("\n[----------] Running %u tests from %s\n", suite->case_count, suite->suite_name);

    /* Run suite setup if provided */
    if (suite->suite_setup != NULL) {
        suite->suite_setup();
    }

    /* Run each test case */
    for (uint32_t i = 0; i < suite->case_count; i++) {
        test_result_t result = run_test_case(&suite->cases[i]);

        g_stats.total++;
        if (result == TEST_RESULT_PASS) {
            g_stats.passed++;
        } else if (result == TEST_RESULT_FAIL) {
            g_stats.failed++;
        } else {
            g_stats.skipped++;
        }
    }

    /* Run suite teardown if provided */
    if (suite->suite_teardown != NULL) {
        suite->suite_teardown();
    }

    OSAL_Printf("[----------] %u tests from %s\n", suite->case_count, suite->suite_name);

    return OS_SUCCESS;
}

/**
 * Run all registered tests
 */
int32_t libtest_run_all(void)
{
    uint32_t suite_count = 0;
    const test_suite_t **suites = libtest_get_all_suites(&suite_count);

    OSAL_Printf("\n[==========] Running %u test suites\n", suite_count);

    libtest_reset_stats();

    for (uint32_t i = 0; i < suite_count; i++) {
        run_suite(suites[i]);
    }

    OSAL_Printf("\n[==========] %u tests from %u test suites ran\n", g_stats.total, suite_count);
    OSAL_Printf("[  PASSED  ] %u tests\n", g_stats.passed);

    if (g_stats.failed > 0) {
        OSAL_Printf("[  FAILED  ] %u tests\n", g_stats.failed);
    }

    if (g_stats.skipped > 0) {
        OSAL_Printf("[ SKIPPED  ] %u tests\n", g_stats.skipped);
    }

    return (g_stats.failed == 0) ? OS_SUCCESS : OS_ERROR;
}

/**
 * Run tests from a specific layer
 */
int32_t libtest_run_layer(const str_t *layer_name)
{
    if (layer_name == NULL) {
        return OS_INVALID_POINTER;
    }

    const test_suite_t *suites[MAX_SUITES];
    uint32_t count = libtest_get_suites_by_layer(layer_name, suites, MAX_SUITES);

    if (count == 0) {
        OSAL_Printf("No tests found for layer: %s\n", layer_name);
        return OS_ERROR;
    }

    OSAL_Printf("\n[==========] Running %u test suites from layer %s\n", count, layer_name);

    libtest_reset_stats();

    for (uint32_t i = 0; i < count; i++) {
        run_suite(suites[i]);
    }

    OSAL_Printf("\n[==========] %u tests from %u test suites ran\n", g_stats.total, count);
    OSAL_Printf("[  PASSED  ] %u tests\n", g_stats.passed);

    if (g_stats.failed > 0) {
        OSAL_Printf("[  FAILED  ] %u tests\n", g_stats.failed);
    }

    return (g_stats.failed == 0) ? OS_SUCCESS : OS_ERROR;
}

/**
 * Run tests from a specific module
 */
int32_t libtest_run_module(const str_t *module_name)
{
    if (module_name == NULL) {
        return OS_INVALID_POINTER;
    }

    const test_suite_t *suites[MAX_SUITES];
    uint32_t count = libtest_get_suites_by_module(module_name, suites, MAX_SUITES);

    if (count == 0) {
        OSAL_Printf("No tests found for module: %s\n", module_name);
        return OS_ERROR;
    }

    OSAL_Printf("\n[==========] Running %u test suites from module %s\n", count, module_name);

    libtest_reset_stats();

    for (uint32_t i = 0; i < count; i++) {
        run_suite(suites[i]);
    }

    OSAL_Printf("\n[==========] %u tests from %u test suites ran\n", g_stats.total, count);
    OSAL_Printf("[  PASSED  ] %u tests\n", g_stats.passed);

    if (g_stats.failed > 0) {
        OSAL_Printf("[  FAILED  ] %u tests\n", g_stats.failed);
    }

    return (g_stats.failed == 0) ? OS_SUCCESS : OS_ERROR;
}

/**
 * Run a specific test suite
 */
int32_t libtest_run_suite(const str_t *suite_name)
{
    if (suite_name == NULL) {
        return OS_INVALID_POINTER;
    }

    const test_suite_t *suite = libtest_find_suite(suite_name);
    if (suite == NULL) {
        OSAL_Printf("Test suite not found: %s\n", suite_name);
        return OS_ERROR;
    }

    libtest_reset_stats();
    run_suite(suite);

    OSAL_Printf("\n[  PASSED  ] %u tests\n", g_stats.passed);

    if (g_stats.failed > 0) {
        OSAL_Printf("[  FAILED  ] %u tests\n", g_stats.failed);
    }

    return (g_stats.failed == 0) ? OS_SUCCESS : OS_ERROR;
}

/**
 * Run a specific test case
 */
int32_t libtest_run_test(const str_t *suite_name, const str_t *test_name)
{
    if (suite_name == NULL || test_name == NULL) {
        return OS_INVALID_POINTER;
    }

    const test_suite_t *suite = libtest_find_suite(suite_name);
    if (suite == NULL) {
        OSAL_Printf("Test suite not found: %s\n", suite_name);
        return OS_ERROR;
    }

    /* Find test case */
    const test_case_t *test = NULL;
    for (uint32_t i = 0; i < suite->case_count; i++) {
        if (OSAL_Strcmp(suite->cases[i].name, test_name) == 0) {
            test = &suite->cases[i];
            break;
        }
    }

    if (test == NULL) {
        OSAL_Printf("Test case not found: %s\n", test_name);
        return OS_ERROR;
    }

    libtest_reset_stats();

    test_result_t result = run_test_case(test);

    g_stats.total = 1;
    if (result == TEST_RESULT_PASS) {
        g_stats.passed = 1;
    } else {
        g_stats.failed = 1;
    }

    return (result == TEST_RESULT_PASS) ? OS_SUCCESS : OS_ERROR;
}

/**
 * Get test statistics
 */
const test_stats_t* libtest_get_stats(void)
{
    return &g_stats;
}

/**
 * Reset test statistics
 */
void libtest_reset_stats(void)
{
    OSAL_Memset(&g_stats, 0, sizeof(test_stats_t));
}
