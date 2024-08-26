
/**
  ******************************************************************************
  * @file    cycles_eval.h
  * @author  IMCv2 Team
  * @brief   Functions to evaluate the number of cycles spent in key logic 
  *          software components in this solution.
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
#ifndef _TEST_CYCLES_EVAL_H
#define _TEST_CYCLES_EVAL_H

#include <stdint.h>

/**
 * @brief Types of cycle tests to execute.
 */
typedef enum
{
    CYCLES_EVAL_USELESS = 0, /*!< Execute the basic 'hal_useless_function()'. */
    CYCLES_EVAL_MSGQ,        /*!< Execute message queue operations. */
    CYCLES_EVAL_STD_MEMCPY,  /*!< Execute stdlib memcpy() test. */
    CYCLES_EVAL_HAL_MEMCPY,  /*!< Execute hal hal_memcpy() test. */
    CYCLES_EVAL_INVALID      /*!< Invalid test case, used as a sentinel value. */

} cycles_test;

/** @addtogroup Exported_CYCLES_EVAL_Functions Cycles test API Exported Functions
 * @{
 */

/**
 * @brief Executes one of the implemented cycle counting tests and returns the 
 *        cycles associated with the test.
 *
 * @param test       The test to execute, specified as a `cycles_test` enum.
 * @param iterations The number of iterations to run the test. If `iterations` 
 *                   is less than or equal to 0, it defaults to 1. 
 * @return The average number of cycles measured, or 0 if an error occurs.
 */

uint64_t run_cycles_test(cycles_test test, int8_t iterations);

/**
  * @}
  */

#endif /* _TEST_CYCLES_EVAL_H */