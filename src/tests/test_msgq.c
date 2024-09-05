
/**
  ******************************************************************************
  * @file    test_msgq.c
  * @author  IMCv2 Team
  * @brief   Measure cycles associated with Message Queue (MessageQ) operations.
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

uintptr_t g_msgq_handle = 0;

/**
 * @brief Measures the number of cycles spent when requesting and releasing buffers
 *        from the message queue using `hal_measure_cycles()`.
 * @param msgq_handle Handle to a previously created message queue using 
 *        msgq_create().
 *
 * @return None.
 */

void test_exec_msgq(uintptr_t unused)
{
    const int frames_count = 1;

    void *p_bufs[1];
    int   ret_val;

    /* Request n frames */
    for ( int i = 0; i < frames_count; i++ )
    {
        p_bufs[i] = msgq_request(g_msgq_handle, 16);
        assert(p_bufs[i] != NULL);
    }

    /* Release frames */
    for ( int i = 0; i < frames_count; i++ )
    {
        ret_val = msgq_release(g_msgq_handle, p_bufs[i]);
        assert(ret_val == 0);
    }
}

/**
 * @brief Create a dummy message Q for this test.
 * @param unussed
 * @return 1 on error.
 */

int test_msgq_prologue(uintptr_t arg)
{
    /* Pool 32 messages of 32 bytes */
    if ( g_msgq_handle == 0 )
    {
        g_msgq_handle = msgq_create(32, 32);
        if ( g_msgq_handle != 0 )
            return 0;
    }

    return 1; /* Error */
}

/**
 * @brief Provides a description for the 'message queue' test.
 * 
 * This function returns a description of the 'message queue' (msgq) test, either 
 * as a brief one-liner or as a more detailed explanation depending on the value 
 * of `description_type`.
 * 
 * @param description_type Specifies the type of description:
 *                         0 for a brief one-line description,
 *                         1 for an in-depth test description.
 * @return A pointer to a string containing the description.
 */

char *test_msgq_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Single insertion and retrieval of a 16-byte buffer from the message queue.";
    }
    else
    {
        return "This test evaluates the performance of a message queue (msgq) system, \n"
               "focusing on the insertion and retrieval of a 16-byte buffer. The test \n"
               "utilizes a free/busy queue that avoids the overhead of dynamic memory \n"
               "allocation by pre-allocating a fixed number of items. These items are \n"
               "managed within two lists: one for busy (in-use) items and one for free \n"
               "(available) items. The test aims to demonstrate the efficiency and \n"
               "deterministic behavior of the queue under controlled conditions.\n";
    }
}
