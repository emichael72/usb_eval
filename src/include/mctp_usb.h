
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

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _MCTP_USB_H
#define _MCTP_USB_H

/* Exported macro ------------------------------------------------------------*/
/** @defgroup MCTP_USB_Exported_MACRO MCTP over USB Macros
  * @{
  */

#define MCTP_USB_MSGQ_MAX_FRAME_SIZE    128  /**< Maximum size in bytes for each 
                                                 allocated buffer in the message queue */
#define MCTP_USB_MSGQ_ALLOCATED_FRAMES  64   /**< Total number of allocated frames 
                                                 in the message queue */

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

void mctp_usb_init(void);


/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 *
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t mctp_usb_get_msgq_handle(void);

/**
  * @}
  */


#endif /* _MCTP_USB_H */

