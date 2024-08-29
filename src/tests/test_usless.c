
/**
  ******************************************************************************
  * @file    test_useless.c
  * @author  IMCv2 Team
  * @brief   Executes a HAL function that is known to take a discrete number 
  *          of cycles.
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

/**
 * @brief Provides a description for the 'useless function' test.
 * 
 * This function returns a description of the 'useless function' test, either as 
 * a brief one-liner or as a more detailed explanation depending on the value 
 * of `description_type`.
 * 
 * @param description_type Specifies the type of description:
 *                         0 for a brief one-line description,
 *                         1 for an in-depth test description.
 * @return A pointer to a string containing the description.
 */

char *test_useless_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Executes a function designed to complete in 5 cycles.";
    }
    else
    {
        return "This test focuses on precise cycle measurements by executing a function \n"
               "that is specifically designed to run in 5 CPU cycles. The function, \n"
               "`hal_useless_function`, consists of 5 NOP (No Operation) instructions, \n"
               "each of which typically takes a single cycle to execute on most architectures.\n"
               "The purpose of this test is to provide a baseline for cycle counting and to verify \n"
               "that the cycle measurement tools and techniques being used are accurate and \n"
               "consistent. By knowing exactly how many cycles the function should take, we can \n"
               "compare the measured cycle count to the expected count, allowing us to detect any \n"
               "discrepancies in the measurement process.\n\n"
               "Here is the implementation of the function:\n\n"
               "void inline hal_useless_function(uintptr_t unused)\n"
               "{\n"
               "    __asm__ __volatile__(\"nop\\n\"\n"
               "                         \"nop\\n\"\n"
               "                         \"nop\\n\"\n"
               "                         \"nop\\n\"\n"
               "                         \"nop\\n\");\n"
               "}\n\n"
               "Each NOP instruction is a no-op that consumes a single cycle, making this \n"
               "function ideal for testing the precision of cycle-based performance measurements.\n";
    }
}
