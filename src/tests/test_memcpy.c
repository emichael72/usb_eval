
/**
  ******************************************************************************
  * @file    test_memcpy.c
  * @author  IMCv2 Team
  * @brief   Measure cycles associated with memory copying operations.
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
#include <string.h>

/**
 * @brief Measures the number of cycles spent on memory copying operations.
 * @param use_hal If 1, the function will use `hal_memcpy()`. Otherwise, it 
 *        defaults to the Xtensa-provided `memcpy()`.
 * @return None.
 */

void test_exec_memcpy(uintptr_t use_hal)
{
    /* 32-byte buffer */
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
}

/**
 * @brief Provides a description for the 'memcpy' test.
 * 
 * This function returns a description of the 'memcpy' test, either as a brief 
 * one-liner or as a more detailed explanation depending on the value of 
 * `description_type`.
 * 
 * @param description_type Specifies the type of description:
 *                         0 for a brief one-line description,
 *                         1 for an in-depth test description.
 * @return A pointer to a string containing the description.
 */

char *test_memcpy_desc_xtensa(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Xtensa memcpy() using 32-byte buffers.";
    }
    else
    {
        return "This test evaluates the performance of the standard Xtensa memcpy() \n"
               "implementation provided by the Xtensa libc. The test is conducted using \n"
               "32-byte buffers, comparing its performance to a custom memcpy function. \n"
               "The focus is on cycle counts and efficiency for small memory operations, \n"
               "providing insights into how well the vendor-supplied memcpy performs \n"
               "under these specific conditions.\n";
    }
}

char *test_memcpy_desc_hal(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Custom optimized memcpy() using 32-byte buffers.";
    }
    else
    {
        return "This test evaluates the performance of a custom, optimized memcpy() \n"
               "function, designed to outperform standard library functions in terms of \n"
               "cycle counts and efficiency. The test is conducted using 32-byte buffers, \n"
               "comparing this custom memcpy against the Xtensa libc memcpy. The results \n"
               "highlight the advantages of a carefully tuned memcpy for specific use cases, \n"
               "with an emphasis on optimizing performance for small memory operations.\n";
    }
}