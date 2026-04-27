/**
 * @file main.c
 * @brief Unified test runner - main entry point
 *
 * Aggregates all module tests and provides CLI interface
 */

#include "tests_core.h"
#include "osal.h"

int main(int argc, char *argv[])
{
    /* Parse command line arguments */
    if (argc == 1) {
        /* No arguments - interactive menu */
        return libutest_interactive_menu();
    }

    /* Run all tests */
    if (OSAL_Strcmp(argv[1], "-a") == 0 || OSAL_Strcmp(argv[1], "--all") == 0) {
        return libutest_run_all();
    }

    /* Run tests by layer */
    if (OSAL_Strcmp(argv[1], "-L") == 0 && argc >= 3) {
        return libutest_run_layer(argv[2]);
    }

    /* Run tests by module */
    if (OSAL_Strcmp(argv[1], "-m") == 0 && argc >= 3) {
        return libutest_run_module(argv[2]);
    }

    /* Run specific test suite */
    if (OSAL_Strcmp(argv[1], "-s") == 0 && argc >= 3) {
        return libutest_run_suite(argv[2]);
    }

    /* List all tests */
    if (OSAL_Strcmp(argv[1], "-l") == 0 || OSAL_Strcmp(argv[1], "--list") == 0) {
        libutest_list_all();
        return 0;
    }

    /* Interactive menu (default) */
    if (OSAL_Strcmp(argv[1], "-i") == 0 || OSAL_Strcmp(argv[1], "--interactive") == 0) {
        return libutest_interactive_menu();
    }

    /* Help */
    if (OSAL_Strcmp(argv[1], "-h") == 0 || OSAL_Strcmp(argv[1], "--help") == 0) {
        OSAL_Printf("\nPMC-BSP Unit Test Runner\n");
        OSAL_Printf("========================\n\n");
        OSAL_Printf("Usage: %s [options]\n\n", argv[0]);
        OSAL_Printf("Options:\n");
        OSAL_Printf("  -a, --all              Run all tests\n");
        OSAL_Printf("  -L <layer>             Run tests from specific layer (OSAL, HAL, PDL)\n");
        OSAL_Printf("  -m <module>            Run tests from specific module\n");
        OSAL_Printf("  -s <suite>             Run specific test suite\n");
        OSAL_Printf("  -l, --list             List all available tests\n");
        OSAL_Printf("  -i, --interactive      Interactive menu (default)\n");
        OSAL_Printf("  -h, --help             Show this help message\n\n");
        OSAL_Printf("Examples:\n");
        OSAL_Printf("  %s                     # Interactive menu\n", argv[0]);
        OSAL_Printf("  %s -a                  # Run all tests\n", argv[0]);
        OSAL_Printf("  %s -L OSAL             # Run all OSAL tests\n", argv[0]);
        OSAL_Printf("  %s -m osal             # Run all osal module tests\n", argv[0]);
        OSAL_Printf("  %s -s osal_task        # Run osal_task test suite\n", argv[0]);
        OSAL_Printf("  %s -l                  # List all tests\n\n", argv[0]);
        return 0;
    }

    /* Unknown option */
    OSAL_Printf("Unknown option: %s\n", argv[1]);
    OSAL_Printf("Use -h or --help for usage information\n");
    return 1;
}
