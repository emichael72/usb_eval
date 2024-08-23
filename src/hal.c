
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


/**
 * @brief HAL session structure for managing emulator and system context.
 * 
 * This structure encapsulates various aspects of the HAL session, including 
 * external input arguments, thread management, memory allocation context, 
 * and system tick management.
 */

typedef struct _hal_session_t
{
    char **argv;                   /**< Array of input arguments passed to the emulator */
    uint8_t *initial_thread_stack; /**< Pointer to the stack of the initial thread */
    uintptr_t pool_ctx;            /**< Context for the global memory pool */
    uint64_t ticks;                /**< System ticks since the epoch */
    uint32_t tick_period;          /**< Period for the ticks timer */
    int argc;                      /**< Count of arguments passed at startup */
    XosTimer ticks_timer;          /**< Handle for the XOS ticks timer */
    XosThread initial_thread;      /**< Handle for the initial XOS thread */

} hal_session;

/* Persistent 'hal' related variables */
hal_session *p_hal = NULL;

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
  uint64_t cycles_before = 0;
  uint64_t cycles_after = 0;
  unsigned int old_int_level;

  /* Disable interrupts */
  old_int_level = xos_disable_interrupts();

  /* Measure the overhead of the entire operation including assignment */
  cycles_before = xt_iss_cycle_count();
  cycles_after = xt_iss_cycle_count();

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

void hal_terminate_simulation(int status) {
    exit(status);  /* Standard C library exit */
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

void hal_useless_function(void) {
    __asm__ __volatile__ (
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
    );
}

/**
 * @brief Retrieve the current system tick count.
 *
 * This function returns the current value of `hal_ticks`, which represents
 * the number of milliseconds that have elapsed since the system started.
 *
 * @return The current system tick count in milliseconds.
 */

uint64_t hal_get_ticks(void) {
  return p_hal->ticks;
}

/**
 * @brief HAL wrapper around the underlying 'brk' style allocator to 
 *        provide a malloc() style API. Note that memory cannot be 
 *        freed in this platform.
 * 
 * @param size The size in bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */

void *hal_alloc(size_t size) {
    if (p_hal && p_hal->pool_ctx) {
        return hal_brk_alloc(p_hal->pool_ctx, size);
    }

    return NULL;
}

/**
 * @brief Measure the number of cycles taken by a given function.
 *
 * This function measures the number of cycles taken to execute the
 * provided function, accounting for and subtracting the overhead
 * measured by `hal_get_sim_overhead_cycles()`.
 *
 * @param func The function whose execution cycles are to be measured.
 * @return The number of cycles taken by the function, or 0 if `func` is NULL.
 */

uint64_t hal_measure_cycles(hal_sim_func func)
{
  uint64_t cycles_before = 0;
  uint64_t cycles_after = 0;
  uint64_t calculated_cycles = 0;
  uint64_t overhead = 0;
  unsigned int old_int_level = 0;

  /* Make sure we have something to work with */
  if (func == NULL)
  {
    return 0;
  }

  /* Get the simulator overhead cycles for precise measurements. */
  overhead = hal_get_sim_overhead_cycles();

  /* Disable interrupts */
  old_int_level = xos_disable_interrupts();

  /* Read initial cycles count */
  cycles_before = xt_iss_cycle_count();

  /* Invoke measured function */
  func();

  /* Get the cycle count after execution */
  cycles_after = xt_iss_cycle_count();

  /* Restore the previous interrupt level to re-enable interrupts */
  xos_restore_interrupts(old_int_level);

  /* Calculate the actual cycles taken by the function */
  calculated_cycles = (cycles_after - cycles_before) - overhead;

  return calculated_cycles;
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

  uint64_t useless_cycles;
  uintptr_t pool_ctx = 0;
  int ret;

  /* Set the system clock frequency */
  xos_set_clock_freq(XOS_CLOCK_FREQ);

  /* Push CCOUNT forward so rollover happens sooner */
  XT_WSR_CCOUNT(HAL_CCOUNT_HACKVAL);
  xos_start_system_timer(-1, 0);

  /* Initilizaes hal basic memory allocator */
  pool_ctx = hal_brk_alloc_init(); 
  assert(pool_ctx != 0);  /* Pool allocation error */

  /* Allocate memory for the hal session variables */
  p_hal = (hal_session*)hal_brk_alloc(pool_ctx, sizeof(hal_session));
  assert(p_hal != NULL);  /* Memory allocation error */

  /* Populate our fresh session */
  p_hal->argc = _argc;
  p_hal->argv = _argv;
  p_hal->pool_ctx = pool_ctx;

  /* Initialize the tick timer to fire every 1 ms */
  p_hal->tick_period = xos_msecs_to_cycles(1);
  xos_timer_init(&p_hal->ticks_timer);

  ret = xos_timer_start(&p_hal->ticks_timer, p_hal->tick_period, XOS_TIMER_PERIODIC,
                        hal_systick_timer, NULL);
  /* System ticks timer could not be initialized */
  assert(ret == XOS_OK);

  /* Allocate stack for the initial thread */
  p_hal->initial_thread_stack = (uint8_t *)malloc(HAL_DEFAULT_STACK_SIZE);
  assert(p_hal->initial_thread_stack != NULL);  /* Memory allocation error */

  /* Create initial thread */
  ret = xos_thread_create(&p_hal->initial_thread, NULL, startThread, NULL,
                          "initThread", p_hal->initial_thread_stack,
                          HAL_DEFAULT_STACK_SIZE, 1, NULL, 0);

  /* Initial thread creation error */
  assert(ret == XOS_OK);

  /* Cycle measurement accuracy test: The following code will
   * execute and measure the cycles performed by 'hal_useless_function'.
   * The function body contains 5 nops, which yields 5 instructions; 
   * however, the return value will vary between 6 and 18 depending on
   * optimization level. The extra cycles are attributed to the function 
   * prologue and epilogue overhead.
   */

  useless_cycles = hal_measure_cycles(hal_useless_function);

  printf("\nXOS Starting kernel, usless cycles: %llu\n",useless_cycles); 

  /* Start Kernel which will block. */
  xos_start(0);

  while(1);

}
