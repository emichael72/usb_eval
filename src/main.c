
/**
  ******************************************************************************
  * @file    main.c
  * @author  IMCv2 Team
  * @brief   Playground for facilitating accurate measurement of the operations
  *          required for supporting MCTP over USB.
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

#include <stdio.h>
#include <hal.h>
#include <mctp_usb.h>
#include <cycles_eval.h>

/**
 * @brief Initial startup thread, calls various 'init' functions and sets
 *        the application in motion.
 * This function serves as the initial startup thread, where it calls
 * various initialization functions to set up the application and
 * begin its execution.
 *
 * @param arg    Pointer to the XOS thread arguments.
 * @param unused Unused parameter.
 * 
 * @return Always returns 0.
 */

static int init_thread(void *arg, int32_t unused)
{

    uint64_t measured_cycles = 0;
    int argc = 0;
    char **argv = NULL;

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

    /* Retrieve argc and argv passed to main */
    hal_get_argcv(&argc, &argv);

    /* Initiliozes the transport layrt, this call will asser opn any error*/
    mctp_usb_init();
    printf("MCTP library initialized!\n");

    /* Performs the basic 'usless clock test */
    measured_cycles = run_cycles_test(CYCLES_EVAL_USELESS, 1);
    printf("Usless cycles: %llu\n", measured_cycles);

    hal_terminate_simulation(10);
    
    while ( 1 )
    {   
        /* Loop indefinitely */
        hal_delay_ms(1000);
    }

    return 0;
}

/**
 * @brief System initialization and startup.
 *
 * This is the entry point for initializing the emulated LX7 environment, 
 * starting the background tick timer interrupt, spawning the initial 
 * main thread, and launching  the XOS scheduler. Once this function is 
 * called, the system will be up and running, and control will be handed 
 * over to the XOS kernel.
 *
 * @return This function does not return under normal circumstances, 
 *         as `hal_sys_init` starts the XOS kernel, which takes over 
 *         system control. If it returns, an error has occurred, 
 *         and the return value is 1.
 */

int main(int argc, char **argv)
{

    /* Initialize the system and start the XOS kernel. 
     * This function will block. */

    hal_sys_init(init_thread, argc, argv);

    /* If we reach here, something went wrong */
    return 1;
}
