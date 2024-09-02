
/**
  ******************************************************************************
  * @file    test_defrag_mctplib.c
  * @author  IMCv2 Team
  * @brief   A simple implementation for libmctp that creates a fake 
  *          'USB device' bindings.
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
#include <hal_llist.h>
#include <hal_msgq.h>
#include <test_defrag_mctplib.h>
#include <libmctp-log.h>
#include <string.h>

#define member_size(type, member) (sizeof(((type *) 0)->member))

/* MCTP standard packet along with 64 bytes payload  */
typedef struct mctplib_packet_t
{
    uint8_t version;              /* MCTP version */
    uint8_t dest;                 /* Destination Endpoint ID */
    uint8_t src;                  /* Source Endpoint ID */
    uint8_t message_tag      : 3; /* Message Tag */
    uint8_t tag_owner        : 1; /* Tag Owner */
    uint8_t packet_sequence  : 2; /* Packet Sequence Number */
    uint8_t end_of_message   : 1; /* End of Message Indicator */
    uint8_t start_of_message : 1; /* Start of Message Indicator */
    uint8_t payload[64];

} mctplib_packet;

/**
 * @brief Holds all global variables for the module.
 */

typedef struct test_defrag_mctplib_session_t
{
    struct mctp        *p_mctp;             /* Pointer to the libmctp instance */
    struct mctp_binding binding;            /* libmctp binding container */
    mctp_eid_t          eid;                /* Our Endpoint ID */
    mctp_eid_t          dest_eid;           /* Remote Endpoint ID */
    uintptr_t           msgq_handle;        /* Handle to the message queue */
    uintptr_t           msgq_contex_handle; /* Handle to the message queue dedicated for context buffers */

} test_defrag_mctplib_session;

/* Pointer to the module's session instance */
test_defrag_mctplib_session *p_mctpusb = NULL;

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t test_defrag_mctplib_get_handle(size_t type)
{
#if ( HAL_PTR_SANITY_CHECKS > 0 )
    if ( p_mctpusb == NULL )
    {
        return 0;
    }
#endif

    if ( type == 0 )
    {
        return p_mctpusb->msgq_handle;
    }
    else
        return p_mctpusb->msgq_contex_handle;
}

/**
 * @brief Run MCTP sequence tests over 'fake USB bus'
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

static void test_defrag_mctplib_dummy_rx(uint8_t eid, bool tag_owner, uint8_t msg_tag, void *data, void *msg, size_t len)
{
    /* Do not do anything here, the cycles are counting :) */
#ifdef DEBUG
    printf("Dummy RX receiver got message from %d, %d bytes.\n", eid, len);
#endif
}

void test_exec_defrag_mctplib(uintptr_t arg)
{

    struct mctp_pktbuf *pkt;
    while ( (pkt = (struct mctp_pktbuf *) msgq_get_next(p_mctpusb->msgq_handle, 1, true)) != NULL )
    {
        mctp_bus_rx(&p_mctpusb->binding, pkt);
    }
}

/* libmctp integration prolog  */
int test_defrag_mctplib_prolog(uintptr_t arg)
{
    uint8_t             frgas_count = 0;
    char                color_byte  = 'A';
    struct mctp_pktbuf *pkt;
    mctplib_packet     *p_mctp, *p_last_mctp = NULL;

    /* Pre build about 25 MCTP messages */
    while ( (pkt = mctp_pktbuf_alloc(&p_mctpusb->binding, sizeof(mctplib_packet))) != NULL )
    {

        p_mctp                   = (mctplib_packet *) mctp_pktbuf_hdr(pkt);
        p_mctp->dest             = p_mctpusb->eid;
        p_mctp->src              = p_mctpusb->dest_eid;
        p_mctp->packet_sequence  = frgas_count;
        p_mctp->start_of_message = 0;
        p_mctp->end_of_message   = 0;

        if ( frgas_count == 0 )
            p_mctp->start_of_message = 1; /* Mark MCTP start */

        memset(p_mctp->payload, (int) color_byte, member_size(mctplib_packet, payload));
        frgas_count++;
        color_byte++;

        /* Keep track of last frame */
        p_last_mctp = p_mctp;
    }

    /* Mark last MCTP message */
    p_last_mctp->end_of_message = 1;

    /* Register a dummy receiver */
    return mctp_set_rx_all(p_mctpusb->p_mctp, test_defrag_mctplib_dummy_rx, NULL);
}

/**
 * @brief Provides a description for the 'libmctp' test.
 * 
 * This function returns a description of the 'libmctp' test, either as a brief 
 * one-liner or as a more detailed explanation depending on the value of 
 * `description_type`.
 * 
 * @param description_type Specifies the type of description:
 *                         0 for a brief one-line description,
 *                         1 for an in-depth test description.
 * @return A pointer to a string containing the description.
 */

char *test_defrag_mctplib_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Uses 'libmctp' to register a USB bus and push MCTP packets through it.";
    }
    else
    {
        return "This test utilizes 'libmctp', a part of the OpenBMC project \n"
               "(https://github.com/openbmc/libmctp), which is a stack capable of managing \n"
               "multiple buses, each with its own Endpoint Identifier (EID) and registered \n"
               "transmit/receive device-implemented functions.\n\n"
               "'libmctp' is designed to handle the Management Component Transport Protocol \n"
               "(MCTP), which is used in platform management systems. It allows for the \n"
               "registration of various types of buses, including I2C, SMBus, and USB, each \n"
               "with its own EID and associated devices.\n\n"
               "In this test, a dummy USB bus is created to evaluate 'libmctp's ability to \n"
               "handle MCTP packets arriving in various forms and sequences. The test \n"
               "specifically examines how well 'libmctp' can process these packets, ensuring \n"
               "they are correctly managed across the bus interface. This includes testing \n"
               "the proper handling of different packet sequences, such as single packet \n"
               "messages, multi-packet messages, and handling of packet flags like Start of \n"
               "Message (SOM) and End of Message (EOM).\n\n"
               "The goal of the test is to validate the robustness and correctness of \n"
               "'libmctp's implementation in scenarios that simulate real-world data traffic \n"
               "on a USB bus, ensuring it can reliably handle MCTP communications under \n"
               "various conditions.\n";
    }
}

/**
 * @brief Initializes the MCTP over USB libmctp bus.
 *
 * This function initializes the MCTP over USB transport layer by allocating 
 * necessary resources and initializing components. The function does not return 
 * anything but will assert if any component fails to initialize correctly.
 *
 * The following steps are performed:
 * - Allocate memory for the MCTP USB session.
 * - Create a message queue pool to be used with libmctp, allowing buffers to be 
 *   requested and released via msgq_request() and msgq_release().
 * - Initialize libmctp and assert if initialization fails.
 */

int test_defrag_mctplib_init(uintptr_t arg)
{
    int ret;

    if ( p_mctpusb != NULL )
        return 1;

    /* Request RAM for this module, assert on failure */
    p_mctpusb = hal_alloc(sizeof(test_defrag_mctplib_session));
    assert(p_mctpusb != NULL);

    /* 
     * Create message queue pool to be used with libmctp. Later, we can use 
     * msgq_request() and msgq_release() to manage buffers in the free-busy list 
     * managed by the 'msgq' module.
     */

    /* Pool for MCTP packets */
    p_mctpusb->msgq_handle = msgq_create(MCTP_USB_MSGQ_MAX_FRAME_SIZE, MCTP_USB_MSGQ_ALLOCATED_FRAMES);
    assert(p_mctpusb->msgq_handle != 0);

    /* Pool for MCTP context buffers */
    p_mctpusb->msgq_contex_handle = msgq_create(MCTP_USB_MAX_CONTEXT_SIZE, MCTP_USB_MSGQ_ALLOCATED_CONTEXES);
    assert(p_mctpusb->msgq_contex_handle != 0);

    /* Initialize libmctp, assert on error. */
    p_mctpusb->p_mctp = mctp_init();
    assert(p_mctpusb->p_mctp != NULL);

    p_mctpusb->eid      = (int) MCTP_USB_SRC_EID;
    p_mctpusb->dest_eid = (int) MCTP_USB_DST_EID;

    mctp_set_max_message_size(p_mctpusb->p_mctp, MCTP_USB_MSGQ_MAX_FRAME_SIZE);

#ifdef DEBUG
    mctp_set_log_stdio(MCTP_LOG_DEBUG);
#endif

    /* Binding */
    p_mctpusb->binding.name        = "USB";
    p_mctpusb->binding.version     = 1;
    p_mctpusb->binding.tx          = NULL;
    p_mctpusb->binding.pkt_size    = MCTP_BODY_SIZE(MCTP_USB_MSGQ_MAX_FRAME_SIZE) - 16;
    p_mctpusb->binding.pkt_header  = 0;
    p_mctpusb->binding.pkt_trailer = 0;

    ret = mctp_register_bus(p_mctpusb->p_mctp, &p_mctpusb->binding, p_mctpusb->eid);
    assert(ret == 0);

    return ret;
}
