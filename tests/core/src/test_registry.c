/**
 * @file test_registry.c
 * @brief Test suite registry implementation
 *
 * Maintains a global registry of all test suites registered via
 * constructor attributes. Provides lookup and enumeration functions.
 */

#include "tests_core.h"
#include "osal.h"

#define MAX_SUITES 128

/* Global registry */
static const test_suite_t *g_registered_suites[MAX_SUITES];
static uint32_t g_suite_count = 0;

/**
 * Register a test suite
 * Called automatically by constructor attributes
 */
void libutest_register_suite(const test_suite_t *suite)
{
    if (NULL == suite) {
        return;
    }

    if (g_suite_count < MAX_SUITES) {
        g_registered_suites[g_suite_count++] = suite;
    } else {
        OSAL_Printf("ERROR: Maximum number of test suites (%d) exceeded\n", MAX_SUITES);
    }
}

/**
 * Get all registered suites
 */
const test_suite_t** test_get_all_suites(uint32_t *count)
{
    if (NULL != count) {
        *count = g_suite_count;
    }
    return g_registered_suites;
}

/**
 * Find suite by name
 */
const test_suite_t* test_find_suite(const str_t *name)
{
    if (NULL == name) {
        return NULL;
    }

    for (uint32_t i = 0; i < g_suite_count; i++) {
        if (OSAL_Strcmp(g_registered_suites[i]->suite_name, name) == 0) {
            return g_registered_suites[i];
        }
    }
    return NULL;
}

/**
 * Get suites by layer
 */
uint32_t test_get_suites_by_layer(const str_t *layer_name, const test_suite_t **suites, uint32_t max_suites)
{
    if (layer_name == NULL || suites == NULL) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < g_suite_count && count < max_suites; i++) {
        if (OSAL_Strcmp(g_registered_suites[i]->layer_name, layer_name) == 0) {
            suites[count++] = g_registered_suites[i];
        }
    }
    return count;
}

/**
 * Get suites by module
 */
uint32_t test_get_suites_by_module(const str_t *module_name, const test_suite_t **suites, uint32_t max_suites)
{
    if (module_name == NULL || suites == NULL) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < g_suite_count && count < max_suites; i++) {
        if (OSAL_Strcmp(g_registered_suites[i]->module_name, module_name) == 0) {
            suites[count++] = g_registered_suites[i];
        }
    }
    return count;
}

/**
 * Get unique layer names
 */
uint32_t test_get_layers(const str_t **layers, uint32_t max_layers)
{
    if (NULL == layers) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < g_suite_count && count < max_layers; i++) {
        const str_t *layer = g_registered_suites[i]->layer_name;

        /* Check if already in list */
        bool found = false;
        for (uint32_t j = 0; j < count; j++) {
            if (OSAL_Strcmp(layers[j], layer) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            layers[count++] = layer;
        }
    }
    return count;
}

/**
 * Get unique module names
 */
uint32_t test_get_modules(const str_t **modules, uint32_t max_modules)
{
    if (NULL == modules) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < g_suite_count && count < max_modules; i++) {
        const str_t *module = g_registered_suites[i]->module_name;

        /* Check if already in list */
        bool found = false;
        for (uint32_t j = 0; j < count; j++) {
            if (OSAL_Strcmp(modules[j], module) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            modules[count++] = module;
        }
    }
    return count;
}
