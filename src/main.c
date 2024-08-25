
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
#include <hal_msgq.h>
#include <libmctp.h>

#define HAL_MCTP_POOL_MAX_FRAME_SIZE        128
#define HAL_MCTP_POOL_ALLOCATED_FRAMES      64

/**
 * @brief Main startup thread.
 * @param arg Pointer to the XOS thread arguments.
 * @param unused Unused parameter.
 * @return Always returns 0.
 */

void execute_msgq_cycles(uintptr_t msgq_handle)
{

    const int frmaes_count = 1;

    msgq_buf *p_buf[HAL_MCTP_POOL_ALLOCATED_FRAMES];
    int ret_val;

    /* Request all the frames */
    for(int i = 0; i < frmaes_count; i++)
    {
        p_buf[i] = msgq_request(msgq_handle,NULL,0,false);   
        assert(p_buf[i] != NULL);   
    }

    /* Release all the frames */
    for(int i = 0; i < frmaes_count; i++)
    {
        ret_val = msgq_release(msgq_handle, p_buf[i]);
        assert(ret_val == 0);   
    }
}

/**
 * @brief Main startup thread.
 * @param arg Pointer to the XOS thread arguments.
 * @param unused Unused parameter.
 * @return Always returns 0.
 */

int init_thread(void *arg, int32_t unused) {

    struct mctp *p_mctp = NULL;
    uintptr_t msgq_handle = 0;
    uint64_t useless_cycles;
    int iters = 0;

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

    /* Create mesgq pool to be used with libmctp, later we could use msgq_request() and
    * and msgq_release() to get / returns buffer to the free-bussy list being manageg 
    * by the 'msgq' module.
    */

    msgq_handle = msgq_create(HAL_MCTP_POOL_MAX_FRAME_SIZE,HAL_MCTP_POOL_ALLOCATED_FRAMES);
    assert(msgq_handle != 0);

    /* Initilizae libmctp, assert on error. */
    p_mctp = mctp_init();
    assert(p_mctp != NULL);

    printf("MCTP library initialized!\n");      
    
    while (1) {
        
        useless_cycles = hal_measure_cycles(execute_msgq_cycles,msgq_handle);
        printf("%02d MsgQ cycles: %llu   \r", iters++, useless_cycles);
        fflush(stdout); 

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

int main(int argc, char **argv) {

    /* Initialize the system and start the XOS kernel. 
     * This function will block. */

    hal_sys_init(init_thread,argc,argv);

    /* If we reach here, something went wrong */
    return 1;
}

