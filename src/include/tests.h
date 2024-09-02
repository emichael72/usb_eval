
/**
  ******************************************************************************
  * @file    tests.h
  * @author  IMCv2 Team
  * @brief   A collection of all of our currently supported tests.
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

#include <hal.h>
#include <test_frag,h>
#include <test_defrag.h>

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _TEST_ALL_H_
#define _TEST_ALL_H_

/**
 * @brief Measures the number of cycles spent on memory copying operations.
 * @param use_hal If 1, the function will use `hal_memcpy()`. Otherwise, it 
 *        defaults to the Xtensa-provided `memcpy()`.
 * @return None.
 */

void  test_exec_memcpy(uintptr_t use_hal);
char *test_memcpy_desc_xtensa(size_t description_type);
char *test_memcpy_desc_hal(size_t description_type);

/**
 * @brief Measures the number of cycles spent when requesting and releasing buffers
 *        from the message queue using `hal_measure_cycles()`.
 * @param msgq_handle Handle to a previously created message queue using 
 *        msgq_create().
 *
 * @return None.
 */

int   test_msgq_prolog(uintptr_t arg);
void  test_exec_msgq(uintptr_t msgq_handle);
char *test_msgq_desc(size_t description_type);

/**
 * @brief Provides a description for the 'useless function' test.
 * @return A pointer to a string containing the description.
 */

char *test_useless_desc(size_t description_type);

#endif /* _TEST_ALL_H_ */
