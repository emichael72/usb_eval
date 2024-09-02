
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
 * @brief Executes the defragmentation test using libmctp.
 * This function simulates the reception of a sequence of fragmented packets,
 * pushes them through the libmctp handler, and expects them to be reassembled and passed to the dummy RX receiver.
 * 
 * @param arg Unused argument, included for compatibility with function pointer expectations.
 */

int   test_defrag_mctplib_prolog(uintptr_t arg);
void  test_exec_defrag_mctplib(uintptr_t arg);
char *test_defrag_mctplib_desc(size_t description_type);

/**
 * @brief Initializes the MCTP fragmentation test environment over USB using libmctp.
 *
 * This function prepares the MCTP over USB transport layer by allocating necessary
 * resources and setting up message queues for packet and context management. It initializes
 * the libmctp library, configures MCTP settings, and registers the MCTP bus with a
 * specified binding. If any allocation fails or if libmctp cannot be initialized,
 * the function returns an error.
 *
 * @param arg Unused parameter, included for function signature compatibility.
 * @return 0 on success, 1 if initialization fails due to resource allocation errors,
 *         or if libmctp fails to initialize.
 */

int test_defrag_mctplib_init(uintptr_t eid);

/**
  * @}
  */

#endif /* _TEST_MCTP_LIB_H_ */
