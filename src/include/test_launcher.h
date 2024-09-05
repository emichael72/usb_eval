
/**
  ******************************************************************************
  * @file    test_launcher.h
  * @author  IMCv2 Team
  * @brief   Allow to register and execute various test and report Cycles 
  *          asociated with them.
  *    
  ******************************************************************************
  * 
  * @copyright
  * @par Copyright (c) 2024 Intel Corporation.
  * All rights reserved.
  * 
  * This code is proprietary to Intel Corporation and may not be used, modified,
  * or distributed without the express written permission of Intel Corporation.
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _TEST_LAUNCHER_H
#define _TEST_LAUNCHER_H

#include <hal.h>
#include <stdint.h>

/* Maximum number of test items */
#define TEST_LAUNCHER_MAX_ITEMS 20

typedef int (*test_launcher_func)(uintptr_t);
typedef char *(*test_launcher_get_description)(size_t description_type);

/**
 * @brief A type that describes a launched test element.
 */
typedef struct test_launcher_item_info_t
{
    test_launcher_func            init;
    test_launcher_func            prologue;     /**< Logic to execute prior to the actual measured function, can be NULL */
    hal_sim_func                  test_func;    /**< Measured function, must not be NULL */
    test_launcher_func            epilogue;     /**< Logic to execute after the measured function, can be NULL */
    test_launcher_get_description desc;         /**< Rerives test decription NULL */
    uintptr_t                     init_arg;     /**< Parameter to pass to the 'prologue' function */
    uintptr_t                     prologue_arg; /**< Parameter to pass to the 'prologue' function */
    uintptr_t                     test_arg;     /**< Parameter to pass to the measured function */
    uintptr_t                     epilogue_arg; /**< Parameter to pass to the 'epilogue' function */
    uint8_t                       repetitions;  /**< Number of repetitions, 1 -> 100 */

} test_launcher_item_info;

/**
  * @}
  */

/**
 * @brief Prints the long description for a registered test at a given index.
 * 
 * This function returns the long description of a test registered in the launcher 
 * at the specified index. If the test or description cannot be found, it returns 
 * an error message.
 * 
 * @param test_index The index of the test whose long description is to be retrieved.
 * @param type 0 for short description, 1 for long.
 * @return A pointer to the description string, or an error message if the test 
 *         or description is not found.
 */

char *test_launcher_get_desc(size_t test_index, size_t type);

/**
 * @brief Prints the description of all registered tests.
 * @return int Always returns 0.
 */

int test_launcher_help(void);

/**
 * @brief Updates a test which si allready registered.
 * @param test_index The index of the test to update.
 * @param item_info A pointer to the test item to be updated.
 * @return int Returns 0 on success, 1 on error.
 */

int test_launcher_update_test(size_t test_index, test_launcher_item_info *item_info);

/**
 * @brief Registers a test item to the launcher.
 * @param tested_item A pointer to the test item to be registered.
 * @return int Returns 0 on success, 1 on error.
 */

int test_launcher_register_test(test_launcher_item_info *tested_item);

/**
 * @brief Executes a registered test based on its index.
 * @param test_index The index of the test to execute.
 * @return int Cycles count associated with the test.
 */

uint64_t test_launcher_execute(size_t test_index);

/**
 * @brief Initializes the test launcher module.
 * @return int Returns 0 on success, 1 on error.
 */

int test_launcher_init(size_t cgi_mode);

/**
  * @}
  */

#endif /* _TEST_LAUNCHER_H */