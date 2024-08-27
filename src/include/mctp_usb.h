
/**
  ******************************************************************************
  * @file    mctp_usb.h
  * @author  IMCv2 Team
  * @brief   A simple implementation for OpenMCTP that creates USB device bindings.
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

#ifndef _MCTP_USB_H
#define _MCTP_USB_H

/* Exported macro ------------------------------------------------------------*/
/** @defgroup MCTP_USB_Exported_MACRO MCTP over USB Macros
  * @{
  */

#define MCTP_USB_APP_NAME "MCTP over USB efficancy evaluation."

/* Define version components */
#define MCTP_USB_APP_VERSION_MAJOR 0
#define MCTP_USB_APP_VERSION_MINOR 1
#define MCTP_USB_APP_VERSION_BUILD 1

/* Concatenate the version components into a single string */
#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)

#ifndef DEBUG
#define MCTP_USB_APP_VERSION STR(MCTP_USB_APP_VERSION_MAJOR) "." STR(MCTP_USB_APP_VERSION_MINOR) "." STR(MCTP_USB_APP_VERSION_BUILD)
#else
#define MCTP_USB_APP_VERSION STR(MCTP_USB_APP_VERSION_MAJOR) "." STR(MCTP_USB_APP_VERSION_MINOR) "." STR(MCTP_USB_APP_VERSION_BUILD) " Debug"
#endif

#define MCTP_USB_MSGQ_MAX_FRAME_SIZE   512 /**< Maximum size in bytes for each allocated buffer in the message queue */
#define MCTP_USB_MSGQ_ALLOCATED_FRAMES 32  /**< Total number of allocated frames in the message queue */

/* Dummy end-point ID used by our bus */
#define MCTP_USB__DEST_EID 9

/**
  * @}
  */

/* Exported API --------------------------------------------------------------*/
/** @defgroup MCTP_USB_Exported_API MCTP over USB Exported API
  * @{
  */

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

void mctp_usb_init(uint8_t eid);

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 *
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t mctp_usb_get_msgq_handle(void);

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

void mctp_usb_run_test_seq(void);

/**
  * @}
  */

#endif /* _MCTP_USB_H */
