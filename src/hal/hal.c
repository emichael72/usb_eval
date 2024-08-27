
/**
 ******************************************************************************
 * @file    hal.c
 * @author  IMCv2 Team.
 * @brief   Basic system routines for the XOS RTOS on the Xtensa LX7.
 *
 * This file contains the essential hardware abstraction layer (HAL) functions
 * necessary for initializing and running the XOS RTOS on the Xtensa LX7 core.
 * It is part of the "MCTP over USB Performance Assessment Project" and is
 * specifically designed to be executed in an emulator environment using
 * `xt-run` rather than on actual hardware.
 *
 * The code leverages the Xtensa SDK and requires the `XTENSA_SYSTEM`
 * environment variable to be correctly defined. This setup is necessary for
 * the SDK to function properly and to ensure that all tools and paths are
 * correctly resolved.
 *
 * @note This code is intended for use with the Xtensa Instruction Set Simulator
 *       (ISS) provided by `xt-run`, and may not behave as expected on real
 *       hardware due to potential differences in the simulation environment.
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
#include <stdio.h>
#include <string.h>

/**
 * @brief HAL session structure for managing emulator and system context.
 *
 * This structure encapsulates various aspects of the HAL session, including
 * external input arguments, thread management, memory allocation context,
 * and system tick management.
 */

typedef struct _hal_session_t
{
    char **   argv;                 /**< Array of input arguments passed to the emulator */
    uint8_t * initial_thread_stack; /**< Pointer to the stack of the initial thread */
    uintptr_t pool_ctx;             /**< Context for the global memory pool */
    uint64_t  ticks;                /**< System ticks since the epoch */
    uint64_t  overhead_cycles;      /**< Pre calculated overhead cycles related to the ISS */
    int       argc;                 /**< Count of arguments passed at startup */
    XosTimer  ticks_timer;          /**< Handle for the XOS ticks timer */
    XosThread initial_thread;       /**< Handle for the initial XOS thread */

} hal_session;

/* Persistent 'hal' related variables */
hal_session *p_hal = NULL;

char *hal_strchr(const char *s, int c)
{
    while ( *s )
    {
        if ( *s == c )
        {
            return (char *) s;
        }
        s++;
    }
    return NULL;
}

/**
 * @brief System 1 millisecond tick timer handler.
 *
 * This function is called by the system timer every 1 millisecond to
 * increment the system tick count. It updates the global variable
 * `ticks` which tracks the number of milliseconds since the system
 * started.
 *
 * @param arg A pointer to user-defined data passed to the timer handler.
 *            This is currently unused.
 */

static void hal_systick_timer(void *arg)
{
    HAL_UNUSED(arg);
    p_hal->ticks++;

/* Auito terminate ?*/
#if defined(HAL_AUTO_TERMINATE) && (HAL_AUTO_TERMINATE > 0)
    if ( p_hal->ticks >= HAL_AUTO_TERMINATE )
    {
        hal_terminate_simulation(1);
    }

#endif
}

/**
 * @brief Parse a single text line into an array of arguments.
 *
 * This function takes a single line of text with arguments separated
 * by a specified separator character (typically a space) and splits
 * it into an array of individual arguments. It also inserts the 
 * program name at index 0 of the argument array and returns the 
 * number of arguments parsed.
 *
 * @param prog_name The program name to be inserted at index 0.
 * @param argv The input string containing the arguments.
 * @param sep The separator character used to delimit arguments.
 * @param argc Pointer to an integer where the argument count will 
 *             be stored.
 * @return A dynamically allocated array of strings representing the 
 *         arguments, or NULL if an error occurs.
 */

char **hal_fix_args(const char *prog_name, const char *argv, char sep, int *argc)
{
    if ( argv == NULL || prog_name == NULL || argc == NULL )
        return NULL;

    *argc = 1;

    /* Count the number of arguments */
    const char *ptr = argv;
    while ( *ptr )
    {
        while ( *ptr && *ptr == sep ) ptr++; /* Skip separators */
        if ( *ptr && *ptr != sep )
        {
            (*argc)++;
            while ( *ptr && *ptr != sep ) ptr++; /* Skip current arg */
        }
    }

    /* Allocate memory for the arguments array */
    char **args = (char **) hal_alloc((*argc + 1) * sizeof(char *));
    if ( args == NULL )
        return NULL;

    /* Insert program name at index 0 */
    args[0] = strdup(prog_name);
    if ( args[0] == NULL )
    {
        return NULL;
    }

    /* Extract arguments */
    int i = 1;
    ptr   = argv;
    while ( *ptr )
    {
        while ( *ptr && *ptr == sep ) ptr++; /* Skip separators */
        if ( *ptr && *ptr != sep )
        {
            const char *start = ptr;
            while ( *ptr && *ptr != sep ) ptr++; /* Find end of arg */

            int len = ptr - start;
            args[i] = (char *) hal_alloc((len + 1) * sizeof(char));
            if ( args[i] == NULL )
            {
                return NULL;
            }

            strncpy(args[i], start, len);
            args[i][len] = '\0'; /* Null-terminate the string */
            i++;
        }
    }
    args[*argc] = NULL; /* Null-terminate the array */

    return args;
}

/**
 * @brief Measure the overhead cycles incurred by calling `xt_iss_cycle_count()`.
 *
 * This function calculates the overhead cycles required to execute the
 * `xt_iss_cycle_count()` function and returns that value, minus one cycle
 * to account for a known simulator discrepancy.
 *
 * @return The overhead cycles measured.
 */

static uint64_t hal_get_sim_overhead_cycles(void)
{
    uint64_t     cycles_before = 0;
    uint64_t     cycles_after  = 0;
    unsigned int old_int_level;

    /* Disable interrupts */
    old_int_level = xos_disable_interrupts();

    /* Measure the overhead of the operation */
    cycles_before = xt_iss_cycle_count();
    cycles_after  = xt_iss_cycle_count();

    /* Restore the previous interrupt level to re-enable interrupts */
    xos_restore_interrupts(old_int_level);

    /* Subtract 1 to account for simulator discrepancy */
    return (cycles_after - cycles_before - 1);
}

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

void inline hal_terminate_simulation(int status)
{
    exit(status); /* Standard C library exit */
}

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

void inline hal_useless_function(uintptr_t xthal_async_L2_region_unlock)
{
    __asm__ __volatile__("nop\n"
                         "nop\n"
                         "nop\n"
                         "nop\n"
                         "nop\n");
}

/**
 * @brief Retrieve the current system tick count.
 *
 * This function returns the current value of `hal_ticks`, which represents
 * the number of milliseconds that have elapsed since the system started.
 *
 * @return The current system tick count in milliseconds.
 */

uint64_t inline hal_get_ticks(void)
{
    return xos_get_system_ticks();
}

/**
 * @brief Copies a memory region from source to destination using the machine's native word size.
 *
 * This function copies a specified number of bytes (`n`) from a source memory region
 * to a destination memory region. The copying is optimized by using the machine's
 * native word size (e.g., 4 bytes on a 32-bit system, 8 bytes on a 64-bit system)
 * for larger chunks, and then copying any remaining bytes one by one.
 *
 * If `HAL_MEMCPY_SANITY_CHECKS` is enabled, the function checks for `NULL` pointers
 * and ensures that `n` is not zero. If any of these conditions are met, the function
 * returns `NULL` without performing the copy.
 *
 * This function is particularly efficient for large memory regions, as it minimizes
 * the number of operations by copying in larger chunks where possible.
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

void inline __attribute__((always_inline)) __attribute__((optimize("-Os"))) * hal_memcpy(void *dest, const void *src, size_t n)
{

#if ( HAL_MEMCPY_SANITY_CHECKS == 1 )
    if ( dest == NULL || src == NULL || n == 0 )
        return NULL;

    size_t word_size = sizeof(uintptr_t);

    /* Check if dest and src 
     * are aligned to the machine's word size */
    if ( ((uintptr_t) dest % word_size != 0) || ((uintptr_t) src % word_size != 0) )
    {
        return NULL; /* Return NULL if pointers are not properly aligned */
    }
#endif

    /* Copy 16-byte chunks */
    while ( n >= 16 )
    {
        *(uint64_t *) dest                   = *(const uint64_t *) src;
        *(uint64_t *) ((uint8_t *) dest + 8) = *(const uint64_t *) ((const uint8_t *) src + 8);
        dest                                 = (uint8_t *) dest + 16;
        src                                  = (const uint8_t *) src + 16;
        n -= 16;
    }

    /* Copy 8-byte chunks */
    while ( n >= 8 )
    {
        *(uint64_t *) dest = *(const uint64_t *) src;
        dest               = (uint8_t *) dest + 8;
        src                = (const uint8_t *) src + 8;
        n -= 8;
    }

    /* Copy 4-byte chunks */
    while ( n >= 4 )
    {
        *(uint32_t *) dest = *(const uint32_t *) src;
        dest               = (uint8_t *) dest + 4;
        src                = (const uint8_t *) src + 4;
        n -= 4;
    }

    /* Copy remaining bytes one by one */
    while ( n > 0 )
    {
        *(uint8_t *) dest = *(const uint8_t *) src;
        dest              = (uint8_t *) dest + 1;
        src               = (const uint8_t *) src + 1;
        n--;
    }

    return dest;
}

/**
 * @brief Efficiently zeroes out a memory region using the machine's native word size.
 *
 * This function zeroes out a memory region by first setting memory in chunks
 * corresponding to the machine's native word size (e.g., 4 bytes on a 32-bit
 * system, 8 bytes on a 64-bit system). It then handles any remaining bytes
 * that do not fit into a full word, ensuring that all `n` bytes are zeroed.
 *
 * This approach improves performance by reducing the number of write operations
 * compared to a byte-by-byte zeroing method, making it particularly effective
 * for larger memory regions.
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

void *hal_zero_buf(void *dest, size_t n)
{
    size_t word_size = sizeof(uintptr_t);

#if ( HAL_MEM_SANITY_CHECKS == 1 )
    if ( dest == NULL || n == 0 )
        return NULL;

    /* Check if dest is aligned to the machine's word size */
    if ( (uintptr_t) dest % word_size != 0 )
    {
        return NULL; /**< Return NULL if the pointer is not properly aligned */
    }
#endif

    uintptr_t *word_ptr = (uintptr_t *) dest;
    uint8_t *  byte_ptr;
    size_t     num_words       = n / word_size;
    size_t     remaining_bytes = n % word_size;

    /* Zero out the memory in word-sized chunks */
    for ( size_t i = 0; i < num_words; i++ )
    {
        *word_ptr++ = 0;
    }

    /* Handle any remaining bytes after the word-sized operations */
    byte_ptr = (uint8_t *) word_ptr;
    for ( size_t i = 0; i < remaining_bytes; i++ )
    {
        *byte_ptr++ = 0;
    }

    return dest;
}

/**
 * @brief HAL wrapper around the underlying 'brk' style allocator to
 *        provide a malloc() style API. Note that memory cannot be
 *        freed in this platform.
 *
 * @param size The size in bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */

void *hal_alloc(size_t size)
{
    if ( p_hal && p_hal->pool_ctx )
    {
        return hal_brk_alloc(p_hal->pool_ctx, size);
    }

    return NULL;
}

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

void inline hal_delay_ms(uint32_t ms)
{
    uint64_t cycles = xos_msecs_to_cycles(ms);
    xos_thread_sleep(cycles);
}

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

uint64_t hal_measure_cycles(hal_sim_func func, uintptr_t arg)
{
    uint64_t     cycles_before     = 0;
    uint64_t     cycles_after      = 0;
    uint64_t     calculated_cycles = 0;
    unsigned int old_int_level     = 0;

    /* Make sure we have something to work with */
    if ( func == NULL )
    {
        return 0;
    }

    /* Enter critical section by disabling interrupts*/
    old_int_level = xos_disable_interrupts();

    /* Read initial cycles count */
    cycles_before = xt_iss_cycle_count();

    /* Invoke measured function */
    func(arg);

    /* Get the cycle count after execution */
    cycles_after = xt_iss_cycle_count();

    /* Restore the previous interrupt level to re-enable interrupts */
    xos_restore_interrupts(old_int_level);

    /* Calculate the actual cycles taken by the function */
    calculated_cycles = (cycles_after - cycles_before) - p_hal->overhead_cycles;

    /* Prevents negative or wrapped-around values from being returned */
    return (calculated_cycles > HAL_OVERHEAD_CYCLES) ? (calculated_cycles - HAL_OVERHEAD_CYCLES) : 0;
}

/**
 * @brief Retrieves the stored argc and argv values from the module session.
 *
 * @param argc Pointer to store the retrieved argc value.
 * @param argv Pointer to store the retrieved argv array.
 * @return 1 if the values are successfully retrieved, 0 if the session is 
 *         not initialized.
 */

int hal_get_argcv(int *argc, char ***argv)
{

    if ( p_hal == NULL )
        return 0;

    if ( argc != NULL && argv != NULL )
    {
        *argc = p_hal->argc;
        *argv = p_hal->argv;
    }

    return 1;
}

/**
 * @brief Initialize the system and start the main application thread.
 *
 * This function performs the initial system setup, including setting up
 * the system timer, initializing the tick timer, and creating the main
 * application thread. It then starts the XOS kernel, which will take over
 * control of the system.
 *
 * @param startThread The entry function for the main application thread.
 * @return never.
 */

__attribute__((noreturn)) void hal_sys_init(XosThreadFunc startThread, int _argc, char **_argv)
{

    uint32_t  tick_period;
    uintptr_t pool_ctx = 0;
    int       ret;

    /* Set the system clock frequency */
    xos_set_clock_freq(XOS_CLOCK_FREQ);

    /* Push CCOUNT forward so rollover happens sooner */
    XT_WSR_CCOUNT(HAL_CCOUNT_HACKVAL);
    xos_start_system_timer(-1, 0);

    /* Initilizaes hal basic memory allocator */
    pool_ctx = hal_brk_alloc_init();
    assert(pool_ctx != 0); /* Pool allocation error */

    /* Allocate memory for the hal session variables */
    p_hal = (hal_session *) hal_brk_alloc(pool_ctx, sizeof(hal_session));
    assert(p_hal != NULL); /* Memory allocation error */

    /* Save allocator context */
    p_hal->pool_ctx = pool_ctx;

    /* TODO: Investigate why arguments are received as a single string instead of an array */
    if ( _argv && _argc >= 1 && _argv[1] && hal_strchr((char *) _argv[1], 0x20) )
    {
        /* Restore arguments structure */
        p_hal->argv = hal_fix_args(_argv[0], (char *) _argv[1], ' ', &p_hal->argc);
    }
    else
    {
        /* Assume that the arguments are already valid and return the original argv */
        p_hal->argv = _argv;
        p_hal->argc = _argc;
    }

    /* Get the simulator overhead cycles for precise measurements. */
    p_hal->overhead_cycles = hal_get_sim_overhead_cycles();

    /* Initialize the tick timer to fire every 1 ms */
    tick_period = xos_msecs_to_cycles(1);
    xos_timer_init(&p_hal->ticks_timer);

    ret = xos_timer_start(&p_hal->ticks_timer, tick_period, XOS_TIMER_PERIODIC, hal_systick_timer, NULL);
    /* System ticks timer could not be initialized */
    assert(ret == XOS_OK);

    /* Allocate stack for the initial thread */
    p_hal->initial_thread_stack = (uint8_t *) hal_brk_alloc(pool_ctx, HAL_DEFAULT_STACK_SIZE);
    assert(p_hal->initial_thread_stack != NULL); /* Memory allocation error */

    /* Create initial thread */
    ret = xos_thread_create(&p_hal->initial_thread, NULL, startThread, NULL, "initThread", p_hal->initial_thread_stack, HAL_DEFAULT_STACK_SIZE, 1, NULL, 0);
    assert(ret == XOS_OK); /* Initial thread creation error */

    setvbuf(stdout, NULL, _IONBF, 0);

    /* Start Kernel which will block. */
    xos_start(0);

    while ( 1 )
        ;
}
