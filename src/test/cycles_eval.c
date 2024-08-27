
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

#include <hal.h>
#include <hal_msgq.h>
#include <mctplib_usb.h>
#include <frag,h>
#include <cycles_eval.h>
#include <string.h> /* stdlib memcpy() */

/**
 * @brief Measures the number of cycles spent when requesting and releasing buffers
 *        from the message queue using `hal_measure_cycles()`.
 * @param msgq_handle Handle to a previously created message queue using 
 *                    `msgq_create()`.
 *
 * @return None.
 */

static void eval_memcpy_cycles(uintptr_t use_hal)
{

    /* 32 bytes buffwe*/
    static const unsigned char src_arr[] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
                                            0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    volatile char dest_arr[sizeof(src_arr)];

    if ( use_hal == 1 )
    {
        hal_memcpy((void *) dest_arr, src_arr, sizeof(dest_arr));
    }
    else
    {
        memcpy((void *) dest_arr, src_arr, sizeof(dest_arr));
    }
};

/**
 * @brief Measures the number of cycles spent when requesting and releasing buffers
 *        from the message queue using `hal_measure_cycles()`.
 * @param msgq_handle Handle to a previously created message queue using 
 *                    `msgq_create()`.
 *
 * @return None.
 */

static void eval_msgq_cycles(uintptr_t msgq_handle)
{

    const int frmaes_count = 1;

    void *p_bufs[MCTP_USB_MSGQ_ALLOCATED_FRAMES];
    int   ret_val;

    /* Request n frames */
    for ( int i = 0; i < frmaes_count; i++ )
    {
        p_bufs[i] = msgq_request(msgq_handle, 16);
        assert(p_bufs[i] != NULL);
    }

    /* Release . frames */
    for ( int i = 0; i < frmaes_count; i++ )
    {
        ret_val = msgq_release(msgq_handle, p_bufs[i]);
        assert(ret_val == 0);
    }
}

/**
 * @brief Executes one of the implemented cycle counting tests and returns the 
 *        cycles associated with the test.
 * @param test       The test to execute, specified as a `cycles_test` enum.
 * @param iterations The number of iterations to run the test. If `iterations` 
 *                   is less than or equal to 0, it defaults to 1. 
 * @return The average number of cycles measured, or 0 if an error occurs.
 */

uint64_t run_cycles_test(cycles_test test, int8_t iterations)
{
    if ( iterations <= 0 )
        iterations = 1;

    uint64_t  total_cycles     = 0;
    uint64_t  avg_cycles       = 0;
    uintptr_t msgq_handle      = mctp_usb_get_msgq_handle();
    char *    test_description = NULL;

    if ( msgq_handle == 0 )
        return 0; /* Could not get a valid handle to the MCTP transport */

    for ( int i = 0; i < iterations; i++ )
    {
        switch ( test )
        {
            case CYCLES_EVAL_USELESS:
                total_cycles += hal_measure_cycles(hal_useless_function, 0);
                test_description = "Function expected to execute in 5";
                break;

            case CYCLES_EVAL_MSGQ:
                total_cycles += hal_measure_cycles(eval_msgq_cycles, msgq_handle);
                test_description = "Message Q (Request / Release)";
                break;

            case CYCLES_EVAL_STD_MEMCPY:
                total_cycles += hal_measure_cycles(eval_memcpy_cycles, 0);
                test_description = "XTensa native memcpy()";
                break;

            case CYCLES_EVAL_HAL_MEMCPY:
                total_cycles += hal_measure_cycles(eval_memcpy_cycles, 1);
                test_description = "Eitan's ultra super optimized memcpy()";
                break;

            case CYCLES_EVAL_USB_BUS_SEQ:
                mctp_usb_run_test_seq();

                printf("<span style=\"color: yellow; font-size: 12px;\">\n");
                printf("\n\nNote: These tests validate the correct handling of MCTP packets over a simulated USB\n");
                printf("bus, ensuring that packet sequences, such as start-of-message (SOM), end-of-message (EOM),\n");
                printf("and sequence numbers, are processed correctly.\n");
                break;

            case CYCLES_EVAL_FRGA:
                if ( frag_test_on_ncsi_rx() == 0 )
                {
                    total_cycles += hal_measure_cycles(frag_test_start, 1);
                    test_description = "NC-SI to MCTP packet fragmentation flow";
                }
                else
                    printf("Error: NC-SI packet was not acquired.\n");
                break;

            default:
                /* Unsupported, show help end exit.*/
                printf("Known test types:\n");
                printf("\t0: Function expected to execute in 5.\n");
                printf("\t1: Message Q (Request / Release).\n");
                printf("\t2: Xtensa native memcpy() with 32 bytes.\n");
                printf("\t3: Eitan's ultra super optimized memcpy() with 32 bytes.\n");
                printf("\t4: Run MCTP sequence tests.\n");
                printf("\t5: NC-SI to MCTP packet fragmentation flow.\n");
                return 0; /* Case not handled */
        }
    }

    avg_cycles = (total_cycles / iterations);
    if ( test_description != NULL )
    {
        printf("'%s' (%d iterations) : %llu Cycles.\n", test_description, iterations, avg_cycles);
        fflush(stdout);
    }

    return avg_cycles;
}
