
/**
  ******************************************************************************
  * @file    hal.h
  * @brief   Basic system routines for XOS RTOS on the Xtensa LX7 emulator.
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
#ifndef _HAL_LX7_H
#define _HAL_LX7_H

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
 * @brief  HAL generic definitions and macros.
 */

#define HAL_BIT(x)               (1 << (x))                 /**< Bit value */
#define HAL_IS_BIT_SET(REG, BIT) (((REG) & (BIT)) == (BIT)) /**< Bit manipulation */
#define HAL_IS_BIT_CLR(REG, BIT) (((REG) & (BIT)) == 0U)    /**< Bit manipulation */
#define HAL_SET_BIT(REG, BIT)    ((REG |= BIT))             /**< Bit manipulation */
#define HAL_CLEAR_BIT(REG, BIT)  ((REG &= ~BIT))            /**< Bit manipulation */
#define HAL_READ_BIT(REG, BIT)   ((REG) & (BIT))            /**< Bit manipulation */
#define HAL_UNUSED(x) \
    (void) (x)        /**< Suppress unused \
                                                                   parameter warnings */
#define __IO volatile /**< IO operation */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

/**
  * @}
  */

#define HAL_APP_NAME "MCTP over USB efficancy evaluation."

/* Define version components */
#define HAL_APP_VERSION_MAJOR 0
#define HAL_APP_VERSION_MINOR 1
#define HAL_APP_VERSION_BUILD 2

/* Concatenate the version components into a single string */
#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

#ifndef DEBUG
#define HAL_APP_VERSION STR(HAL_APP_VERSION_MAJOR) "." STR(HAL_APP_VERSION_MINOR) "." STR(HAL_APP_VERSION_BUILD)
#else
#define HAL_APP_VERSION STR(HAL_APP_VERSION_MAJOR) "." STR(HAL_APP_VERSION_MINOR) "." STR(HAL_APP_VERSION_BUILD) " Debug"
#endif

/**
 * @brief  HAL run-time definitions.
 */

#define HAL_CCOUNT_HACKVAL     (0xFC000000) /**< CCOUNT forward hack value */
#define HAL_DEFAULT_STACK_SIZE (2 * 1024)   /**< Default stack size in bytes for threads */
#define HAL_POOL_SIZE          (32 * 1024)  /**< Bytes available for the inner pool */
#define HAL_AUTO_TERMINATE \
    (60000) /**< Auto exit emulator after n 
                                                     milliseconds */
#define HAL_MEM_SANITY_CHECKS \
    0 /**< Enable sanity checks in 
                                                     hal_memcpy() */
#define HAL_BRK_ALLOC_ZERO_MEM \
    1 /**< Initialize allocated memory 
                                                     to zero */
#define HAL_MSGQ_USE_CRITICAL \
    0 /**< Enables critical sections 
                                                     in the message queue to 
                                                     ensure thread-safe operation 
                                                     across multiple contexts */
#define HAL_MSGQ_SANITY_CHECKS \
    0 /**< Enable sanity checks when requesting
                                                     and releasing messages */

#define HAL_PTR_SANITY_CHECKS 1 /**< Enable generic pointers checks */

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
#define HAL_OVERHEAD_CYCLES (14)
#else
#define HAL_OVERHEAD_CYCLES (11)
#endif

/* Teminal ANSI escapes code for colors support */
#define ANSI_CLS         "\033[2J"
#define ANSI_CLR         "\033[K"
#define ANSI_CURSOR_OFF  "\033[?25l"
#define ANSI_CURSOR_ON   "\033[?25h"
#define ANSI_BLACK       "\033[30m"          /* Black */
#define ANSI_RED         "\033[1;31m"        /* Red */
#define ANSI_GREEN       "\033[1;32m"        /* Green */
#define ANSI_YELLOW      "\033[1;33m"        /* Yellow */
#define ANSI_BLUE        "\033[1;34m"        /* Blue */
#define ANSI_MAGENTA     "\033[1;35m"        /* Magenta */
#define ANSI_CYAN        "\033[1;36m"        /* Cyan  */
#define ANSI_WHITE       "\033[37m"          /* White */
#define ANSI_BOLDBLACK   "\033[1m\033[30m"   /* Bold Black */
#define ANSI_BOLDRED     "\033[1m\033[1;31m" /* Bold Red */
#define ANSI_BOLDGREEN   "\033[1m\033[1;32m" /* Bold Green */
#define ANSI_BOLDYELLOW  "\033[1m\033[1;33m" /* Bold Yellow */
#define ANSI_BOLDBLUE    "\033[1m\033[1;34m" /* Bold Blue */
#define ANSI_BOLDMAGENTA "\033[1m\033[1;35m" /* Bold Magenta */
#define ANSI_BOLDCYAN    "\033[1m\033[1;36m" /* Bold Cyan */
#define ANSI_BOLDWHITE   "\033[1m\033[37m"   /* Bold White */
#define ANSI_BG_BLACK    "\033[0;40m"
#define ANSI_FG_BLACK    "\033[1;30m"
#define ANSI_BG_WHITE    "\033[0;47m"
#define ANSI_FG_WHITE    "\033[1;37m"
#define ANSI_BG_CYAN     "\033[0;46m"
#define ANSI_BG_YELLOW   "\033[0;43m"
#define ANSI_FG_DEFAULT  "\033[0;39m"
#define ANSI_BG_DEFAULT  "\033[0;49m"
#define ANSI_MODE        ANSI_CURSOR_ON ANSI_BG_DEFAULT ANSI_FG_DEFAULT

/**
  * @}
  */

/**
 * @brief Pattern descriptor used for buffer painting and validation.
 */
typedef struct
{
    uint8_t  version;  /**< Version number of the pattern descriptor. */
    uint16_t checksum; /**< Checksum of the pattern data. */

} HAL_PATTERN_DESCRIPTOR;

#define HAL_PATTERN_DESCRIPTOR_SIZE sizeof(HAL_PATTERN_DESCRIPTOR)
#define HAL_MIN_PATTERN_BUFFER_SIZE (HAL_PATTERN_DESCRIPTOR_SIZE + 32)

/** @addtogroup Exported_HAL_Functions HAL Exported Functions
 * @{
 */

/**
 * @brief Defines a prototype for a function whose execution cycles are measured.
 */

typedef void (*hal_sim_func)(uintptr_t);

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
 * @brief Copies a memory region from source to destination using the machine's native word size.
 *
 * This function copies a specified number of bytes (`n`) from a source memory region
 * to a destination memory region. The copying is optimized by using the machine's
 * native word size (e.g., 4 bytes on a 32-bit system, 8 bytes on a 64-bit system)
 * for larger chunks, and then copying any remaining bytes one by one.
 *
 * @param dest Pointer to the destination memory region where data will be copied.
 * @param src  Pointer to the source memory region from where data will be copied.
 * @param n    Number of bytes to copy from the source to the destination.
 *
 * @return Pointer to the destination memory region (same as `dest`), or `NULL` if
 *         sanity checks are enabled and an invalid parameter is detected.
 *
 * @note This function assumes that the `dest` and `src` pointers are properly aligned
 *       according to the machine's word size. Misaligned pointers may result in
 *       undefined behavior or reduced performance.
 */

void *hal_memcpy(void *__restrict dest, const void *__restrict src, size_t n);

/**
 * @brief Efficiently zeroes out a memory region using the machine's native word size.
 *
 * This function zeroes out a memory region by first setting memory in chunks
 * corresponding to the machine's native word size (e.g., 4 bytes on a 32-bit
 * system, 8 bytes on a 64-bit system). It then handles any remaining bytes
 * that do not fit into a full word, ensuring that all `n` bytes are zeroed.
 *
 * @param dest Pointer to the start of the memory region to be zeroed.
 * @param n    Number of bytes to zero out in the memory region.
 *
 * @return Pointer to the start of the memory region (same as `dest`).
 *
 * @note This function assumes that the `dest` pointer is properly aligned
 *       according to the machine's word size. Misaligned pointers may
 *       result in undefined behavior.
 */

void *hal_zero_buf(void *dest, size_t n);

/**
 * @brief Outputs a byte array as hex strings to the terminal.
 *
 * This function prints the contents of a byte array in a formatted hex dump
 * style. Each line contains 16 bytes in hexadecimal representation, followed
 * by their ASCII equivalents (or a '.' for non-printable characters). The memory
 * address is optionally included at the start of each line.
 *
 * @param data Pointer to the byte array to be dumped.
 * @param size Number of bytes to be displayed from the array.
 * @param addAddress Boolean flag to include or omit the address field in the output.
 */

void hal_hexdump(const void *data, size_t size, bool addAddress, const char *prefx);

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

void hal_useless_function(uintptr_t arg);

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
 * @param arg Parameter that can be passed to the measured function.
 * @return The number of cycles taken by the function, or 0 if `func` is NULL.
 */

uint64_t hal_measure_cycles(hal_sim_func func, uintptr_t arg);

/**
 * @brief Paints or vlidarte a buffer with a pattern and a descriptor.
 *
 * This function fills the given buffer with a predefined pattern and
 * adds a pattern descriptor at the beginning of the buffer.
 *
 * @param p_buffer Pointer to the buffer to be painted.
 * @param len Length of the buffer in bytes.
 * @return 0 on success, 1 on error (e.g., invalid input).
 */

int hal_paint_buffer(void *p_buffer, size_t len);
int hal_validate_paint_buffer(void *p_buffer, size_t len);

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
 * @brief Retrieves the stored argc and argv values from the module session.
 *
 * @param argc Pointer to store the retrieved argc value.
 * @param argv Pointer to store the retrieved argv array.
 * 
 * @return 1 if the values are successfully retrieved, 0 if the session is 
 *         not initialized.
 */

int hal_get_argcv(int *argc, char ***argv);

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

#endif /* _HAL_LX7_H */
