
/**
  ******************************************************************************
  * @file    mctp_usb.c
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

#include <hal.h>
#include <hal_msgq.h>
#include <libmctp.h>
#include <mctp_usb.h>

/**
 * @brief Holds all global variables for the module.
 */
typedef struct mctp_usb_session_t
{
    struct mctp *p_mctp;      /*!< Pointer to the libmctp instance */
    uintptr_t    msgq_handle; /*!< Handle to the message queue */
} mctp_usb_session;

/* Pointer to the module's session instance */
mctp_usb_session *p_mctpusb = NULL;

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t mctp_usb_get_msgq_handle(void)
{
    if ( p_mctpusb != NULL )
        return p_mctpusb->msgq_handle;

    return 0;
}

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

void mctp_usb_init(void)
{
    /* Request RAM for this module, assert on failure */
    p_mctpusb = hal_alloc(sizeof(mctp_usb_session));
    assert(p_mctpusb != NULL);

    /* Create message queue pool to be used with libmctp. Later, we can use 
     * msgq_request() and msgq_release() to manage buffers in the free-busy list 
     * managed by the 'msgq' module.
     */
    p_mctpusb->msgq_handle = msgq_create(MCTP_USB_MSGQ_MAX_FRAME_SIZE, MCTP_USB_MSGQ_ALLOCATED_FRAMES);
    assert(p_mctpusb->msgq_handle != 0);

    /* Initialize libmctp, assert on error. */
    p_mctpusb->p_mctp = mctp_init();
    assert(p_mctpusb->p_mctp != NULL);
}
