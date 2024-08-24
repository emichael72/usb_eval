
/**
  ******************************************************************************
  * @file    hal.c
  * @brief   Basic system routines for XOS RTOS on the Xtensa LX7 emulator.
  * 
  ******************************************************************************
  * 
  * XOS is a lightweight, real-time operating system (RTOS) specifically designed 
  * for the Xtensa processor architecture. It provides essential multitasking 
  * capabilities, including thread management, timers, and synchronization 
  * mechanisms, making it ideal for embedded systems requiring efficient and 
  * deterministic operation. This file contains the hardware abstraction layer 
  * (HAL) routines tailored for the XOS RTOS running on the Xtensa LX7 emulator.
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
#ifndef _LIBMCTP_USB_LX7_HAL_H
#define _LIBMCTP_USB_LX7_HAL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <xtensa/config/core.h>
#include <xtensa/xos_errors.h>
#include <xtensa/xtbsp.h>
#include <xtensa/xos.h>
#include <xtensa/xos_thread.h>
#include <xtensa/xtbsp.h>
#include <xtensa/xtruntime.h>
#include <xtensa/sim.h>

/* Exported macro ------------------------------------------------------------*/
/** @defgroup HAL_Base_Exported_Macro HAL_Base Exported Macro
  * @{
  */

/**
 * @brief  HAL definitions misfits junk yard.
 */

#define HAL_BIT(x)                          (1 << (x))                      /**!< Bit value */
#define HAL_IS_BIT_SET(REG, BIT)            (((REG) & (BIT)) == (BIT))      /**!< Bits manipulations */
#define HAL_IS_BIT_CLR(REG, BIT)            (((REG) & (BIT)) == 0U)         /**!< Bits manipulations */
#define HAL_SET_BIT(REG, BIT)               ((REG |= BIT))                  /**!< Bits manipulations */
#define HAL_CLEAR_BIT(REG, BIT)             ((REG &= ~BIT))                 /**!< Bits manipulations */
#define HAL_READ_BIT(REG, BIT)              ((REG) & (BIT))                 /**!< Bits manipulations */
#define HAL_UNUSED(x)                       (void)(x)                       /**!< Used to suppress warnings about unused parameters  */
#define __IO    volatile

/**
  * @}
  */

#define HAL_CCOUNT_HACKVAL       0xFC000000  /**< CCOUNT forward hack value */
#define HAL_DEFAULT_STACK_SIZE   (1 * 1024)  /**< Default stack size in bytes for threads */
#define HAL_POOL_SIZE            (8 * 1024)  /**< Bytes avilable for the inner pool */
#define HAL_AUTO_TERMINATE       (60000)     /**< Auto exit emulator after n milliseonds */

/******************************************************************************
  * 
  * The `HAL_OVERHEAD_CYCLES` macro defines the number of overhead cycles 
  * incurred during cycle measurement in the emulator. This overhead stems 
  * from the function prologue and epilogue, which are sequences of 
  * instructions that the compiler inserts at the beginning and end of a 
  * function.
  * 
  * - **Prologue**: Saves the state of registers, sets up the stack frame, 
  *   and performs initializations. In debug mode, the prologue is larger 
  *   due to extra debug code, like saving additional registers.
  * 
  * - **Epilogue**: Restores registers, cleans up the stack, and prepares 
  *   the CPU to return to the caller. In debug mode, it may include extra 
  *   operations, such as stack checks.
  * 
  * The overhead varies by build mode:
  * - **Debug mode**: 13 cycles, due to more extensive prologue/epilogue.
  * - **Release mode**: 10 cycles, with optimized prologue/epilogue.
  * 
  * These values help adjust measured cycles to reflect the actual execution 
  * time, excluding entry and exit overhead.
  * 
  *******************************************************************************/

#ifdef DEBUG
#define HAL_OVERHEAD_CYCLES  (13)
#else
#define HAL_OVERHEAD_CYCLES  (10)
#endif

/** @addtogroup Exported_HAL_Functions HAL Exported Functions
 * @{
 */


/**
 * @brief Defines a prototype for a function whose execution cycles are measured.
 */

typedef void (*hal_sim_func)(void);

/**
 * @brief Initialize an allocation context for managing memory.
 *
 * Prepares an object of type 'hal_brk_ctx' to be used by the sbrk allocator.
 *
 * @param mem_start Pointer to the start of the memory region to be managed.
 * @param tot_size  Size in bytes available for allocation starting from 'mem_start'.
 * @retval Pointer to an initialized memory context or 0 on error.
 */

uintptr_t hal_brk_alloc_init(void);

/**
 * @brief Allocate a memory chunk from a pre-initialized region.
 *
 * This function allocates a memory block from the memory region managed by 
 * a pre-initialized 'hal_brk_ctx' structure.
 *
 * @param ctx Pointer to an initialized sbrk context (using hal_brk_alloc_init()).
 * @param size Size in bytes to allocate, this will be aligned up as needed.
 * @retval Valid pointer or NULL on error.
 */

void *hal_brk_alloc(__IO uintptr_t ctx, size_t size);

/**
 * @brief HAL wrapper around the underlying 'brk' style allocator to 
 *        provide a malloc() style API. Note that memory cannot be 
 *        freed in this platform.
 * 
 * @param size The size in bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */

void *hal_alloc(size_t size);

/**
 * @brief A function used for testing cycle measurement accuracy in the emulator.
 *
 * This function is intended to serve as a simple benchmark for verifying that
 * cycle measurements are accurate within the emulator environment. It contains
 * 5 `nop` (no operation) instructions in inline assembly, which do nothing but
 * consume a small, predictable amount of processing time.
 *
 * @return None (void function).
 */

void hal_useless_function(void);

/**
 * @brief Terminates the simulation and exits the program.
 *
 * This function is a simple wrapper around the standard C library's `exit()` 
 * function. It is used to terminate the simulation and exit the program with 
 * the specified status code. This can be particularly useful when running 
 * simulations in an emulated environment where a clean exit is required.
 *
 * @param status The exit status code to be returned to the operating system.
 *               A status of 0 typically indicates successful completion, while 
 *               any non-zero value indicates an error or abnormal termination.
 */

void hal_terminate_simulation(int status);

/**
 * @brief Measure the number of cycles taken by a given function.
 *
 * This function measures the number of cycles taken to execute the
 * provided function, accounting for and subtracting the overhead
 * measured by `hal_get_sim_overhead_cycles()`.
 *
 * @param func The function whose execution cycles are to be measured.
 * @return The number of cycles taken by the function.
 */

uint64_t hal_measure_cycles(hal_sim_func func);

/**
 * @brief Retrieve the current system tick count.
 *
 * This function returns the current value of `hal_ticks`, which represents
 * the number of milliseconds that have elapsed since the system started.
 *
 * @return The current system tick count in milliseconds.
 */

uint64_t hal_get_ticks(void);

/**
 * @brief Delay execution for a specified number of milliseconds.
 *
 * This function converts the given milliseconds into CPU cycles and 
 * makes the current thread sleep for that duration using XOS API.
 *
 * @param[in] ms  Number of milliseconds to delay the execution.
 *
 * @note This function uses the XOS API to achieve the delay.
 *       The delay is not exact and depends on the system's tick rate 
 *       and CPU clock accuracy.
 */

void hal_delay_ms(uint32_t ms);

/**
 * @brief Initialize the system and start the main application thread.
 *
 * This function performs the initial system setup, including setting up
 * the system timer, initializing the tick timer, and creating the main 
 * application thread. It then starts the XOS kernel, which will take over
 * control of the system.
 *
 * @param startThread The entry function for the main application thread.
 * @return non return call.
 */

void hal_sys_init(XosThreadFunc startThread, int _argc, char **argv);

/**
  * @}
  */


#endif /* _LIBMCTP_USB_LX7_HAL_H */
