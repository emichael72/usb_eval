
/**
  ******************************************************************************
  * @file    cycles_eval.c
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
#ifndef _LIBMCTP_CYCLES_EVAL_H
#define _LIBMCTP_CYCLES_EVAL_H

#include <hal.h>
#include <hal_msgq.h>
#include <mctp_usb.h>
#include <cycles_eval.h>

/**
 * @brief Measures the number of cycles spent when requesting and releasing buffers
 *        from the message queue using `hal_measure_cycles()`.
 *
 * @param msgq_handle Handle to a previously created message queue using 
 *                    `msgq_create()`.
 *
 * @return None.
 */

static void eval_msgq_cycles(uintptr_t msgq_handle)
{

    const int frmaes_count = 1;

    msgq_buf *p_buf[MCTP_USB_MSGQ_ALLOCATED_FRAMES];
    int       ret_val;

    /* Request n frames */
    for ( int i = 0; i < frmaes_count; i++ )
    {
        p_buf[i] = msgq_request(msgq_handle, NULL, 0, false);
        assert(p_buf[i] != NULL);
    }

    /* Release . frames */
    for ( int i = 0; i < frmaes_count; i++ )
    {
        ret_val = msgq_release(msgq_handle, p_buf[i]);
        assert(ret_val == 0);
    }
}

/**
 * @brief Executes one of the implemented cycle counting tests and returns the 
 *        cycles associated with the test.
 *
 * @param test       The test to execute, specified as a `cycles_test` enum.
 * @param iterations The number of iterations to run the test. If `iterations` 
 *                   is less than or equal to 0, it defaults to 1. 
 * @return The average number of cycles measured, or 0 if an error occurs.
 */

uint64_t run_cycles_test(cycles_test test, int8_t iterations)
{
    if (iterations <= 0)
        iterations = 1;

    uint64_t total_cycles = 0;
    uintptr_t msgq_handle = mctp_usb_get_msgq_handle();

    if (msgq_handle == 0)
        return 0; /* Could not get a valid handle to the MCTP transport */

    for (int i = 0; i < iterations; i++)
    {
        switch (test)
        {
            case CYCLES_EVAL_USELESS:
                total_cycles += hal_measure_cycles(hal_useless_function, 0);
                break;

            case CYCLES_EVAL_MSGQ:
                total_cycles += hal_measure_cycles(eval_msgq_cycles, msgq_handle);
                break;

            default:
                return 0; /* Case not handled */
        }
    }

    return total_cycles / iterations;
}


#endif /* _LIBMCTP_CYCLES_EVAL_H */