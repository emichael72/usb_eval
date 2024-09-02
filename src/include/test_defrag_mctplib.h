
/**
  ******************************************************************************
  * @file    test_defrag_mctplib.h
  * @author  IMCv2 Team
  * @brief   A simple implementation for libmctp that creates USB device bindings.
  * 
  * This adaptation is purely for performance assessments, so no 'real USB' 
  * driver is involved. The implementation is intended for benchmarking and 
  * does not interact with actual USB hardware.
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

#include <stdint.h>
#include <stdbool.h>
#include <libmctp.h>

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _TEST_MCTP_LIB_H_
#define _TEST_MCTP_LIB_H_

/* Exported macro ------------------------------------------------------------*/
/** @defgroup MCTP_USB_Exported_MACRO MCTP over USB Macros
  * @{
  */

#define MCTP_USB_MSGQ_MAX_FRAME_SIZE     100  /**< Maximum size in bytes for each allocated buffer in the message queue */
#define MCTP_USB_MSGQ_ALLOCATED_FRAMES   25   /**< Total number of allocated frames in the message queue */
#define MCTP_USB_MAX_CONTEXT_SIZE        1600 /**< Used by context buffer, aqual to max Ethernet frame */
#define MCTP_USB_MSGQ_ALLOCATED_CONTEXES 1    /**<  Count of context buffers */
#define MCTP_USB_SRC_EID                 9    /**< Dummy local end-point ID used by our test */
#define MCTP_USB_DST_EID                 10   /**< Dummy remote end-point ID used by our test */

/**
  * @}
  */

/* Exported API --------------------------------------------------------------*/
/** @defgroup MCTP_USB_Exported_API MCTP over USB Exported API
  * @{
  */

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t test_defrag_mctplib_get_handle(size_t type);

/**
 * @brief Run MCTP sequence tests over USB.
 *
 * This function serves as the entry point for running MCTP (Management Component
 * Transport Protocol) sequence tests specifically designed for the LX7 platform.
 * These tests validate the correct handling of MCTP packets over a simulated USB
 * bus, ensuring that packet sequences, such as start-of-message (SOM), end-of-message
 * (EOM), and sequence numbers, are processed correctly.
 *
 * The function currently runs a series of predefined tests to validate the packet
 * reception logic, checking that packets are reconstructed correctly from the
 * sequence of incoming data. Over time, this function will be extended to implement
 * a complete USB bus for MCTP, allowing real MCTP messages to be transmitted and
 * received over USB.
 *
 * @note This function is part of the MCTP USB implementation on the Xtensa LX7 platform
 * and relies on the libmctp library and the MCTP test utilities.
 *
 * @return void
 */

int   test_defrag_mctplib_prolog(uintptr_t arg);
void  test_exec_defrag_mctplib(uintptr_t arg);
char *test_defrag_mctplib_desc(size_t description_type);

/**
 * @brief Initializes the MTCP over USB transport layer.
 *
 * This function initializes the MTCP over USB transport layer by allocating 
 * necessary resources and initializing components. The function does not return 
 * anything but will assert if any component fails to initialize correctly.
 *
 * The following steps are performed:
 * - Allocate memory for the MTCP USB session.
 * - Create a message queue pool to be used with libmctp, allowing buffers to be 
 *   requested and released via msgq_request() and msgq_release().
 * - Initialize libmctp and assert if initialization fails.
 */

int test_defrag_mctplib_init(uintptr_t eid);

/**
  * @}
  */

#endif /* _TEST_MCTP_LIB_H_ */
