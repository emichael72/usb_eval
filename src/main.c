
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
#include <libmctp.h>

/**
 * @brief Main startup thread.
 *
 * This function is called by the HAL initialization routine at the start of
 * the system. Its current role is to print the number of seconds that have 
 * passed since the system started.
 *
 * @param arg   Pointer to the XOS thread arguments.
 * @param unused Unused parameter.
 * @return Always returns 0.
 */

int start_thread(void *arg, int32_t unused) {

    uint64_t ticks, seconds = 0;
    struct mctp *p_mctp = NULL;

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

    /* Initilizae libmctp, assert on error. */
    p_mctp = mctp_init();
    assert(p_mctp != NULL);

    printf("MCTP library initialized!\n");      
    
    while (1) {
        
        ticks = hal_get_ticks();
        if (ticks % 1000 == 0) {
            printf("Seconds: %llu\r", seconds++);
            fflush(stdout);            
        }
    }

    return 0; 
}

/**
 * @brief System initialization and startup.
 *
 * This is the primary entry point for the application, responsible for 
 * initializing the emulated LX7 environment, starting the background 
 * tick timer interrupt, spawning the initial main thread, and launching 
 * the XOS scheduler. Once this function is called, the system will be 
 * up and running, and control will be handed over to the XOS kernel.
 *
 * @return This function does not return under normal circumstances, 
 *         as `hal_sys_init` starts the XOS kernel, which takes over 
 *         system control. If it returns, an error has occurred, 
 *         and the return value is 1.
 */
int main(int argc, char **argv) {

    /* Initialize the system and start the XOS kernel. This function will block. */
    hal_sys_init(start_thread,argc,argv);

    /* If we reach here, something went wrong */
    return 1;
}

