
/**
  ******************************************************************************
  * @file    test_defrag_mctplib.c
  * @author  IMCv2 Team
  * @brief   Aggregates 25 fragments of MCTP packets into a single frame 
  *          using libmctp.
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
        return p_mctpusb->msgq_handle;
    else
        return p_mctpusb->msgq_contex_handle;
}

/**
 * @brief Executed by libmctp when a complete packet destined for our EID is fully assembled.
 * This function acts as a placeholder to demonstrate reception and processing of MCTP messages.
 *
 * @param eid The endpoint identifier to which the message was sent.
 * @param tag_owner Boolean indicating if the tag is owned.
 * @param msg_tag The message tag.
 * @param data Pointer to the data received.
 * @param msg Pointer to the complete message.
 * @param len Length of the message in bytes.
 */

static void test_defrag_mctplib_dummy_rx(uint8_t eid, bool tag_owner, uint8_t msg_tag, void *data, void *msg, size_t len)
{
#ifdef DEBUG
    printf("Dummy RX receiver got a message from EID %d, length %d bytes.\n", eid, len);
#endif
}

/**
 * @brief Executes the defragmentation test using libmctp.
 * This function simulates the reception of a sequence of fragmented packets,
 * pushes them through the libmctp handler, and expects them to be reassembled and passed to the dummy RX receiver.
 * 
 * @param arg Unused argument, included for compatibility with function pointer expectations.
 */

void test_exec_defrag_mctplib(uintptr_t arg)
{
    struct mctp_pktbuf *pkt;
    while ( (pkt = (struct mctp_pktbuf *) msgq_get_next(p_mctpusb->msgq_handle, 1, true)) != NULL )
    {
        mctp_bus_rx(&p_mctpusb->binding, pkt);
    }
}

/**
 * @brief Sets up the environment for a defragmentation test (libmctp).
 * This prolog function creates and initializes a series of MCTP packets, setting the correct
 * destination ID and sequence, ready to be processed by the libmctp Rx handler.
 * 
 * @param arg Unused argument, included for compatibility with function pointer expectations.
 * @return Status of the operation, 0 on success, non-zero on failure.
 */

int test_defrag_mctplib_prolog(uintptr_t arg)
{
    uint8_t             frgas_count = 0;
    char                color_byte  = 'A';
    struct mctp_pktbuf *pkt;
    mctplib_packet     *p_mctp, *p_last_mctp = NULL;

    /* Pre-build about 25 MCTP messages */
    while ( (pkt = mctp_pktbuf_alloc(&p_mctpusb->binding, sizeof(mctplib_packet))) != NULL )
    {
        p_mctp                   = (mctplib_packet *) MCTP_PKTBUF_HDR(pkt);
        p_mctp->dest             = p_mctpusb->eid;
        p_mctp->src              = p_mctpusb->dest_eid;
        p_mctp->packet_sequence  = frgas_count;
        p_mctp->start_of_message = (frgas_count == 0); // Mark the start of the MCTP message
        p_mctp->end_of_message   = 0;

        memset(p_mctp->payload, (int) color_byte, sizeof(p_mctp->payload));
        frgas_count++;
        color_byte++;

        /* Keep track of the last frame */
        p_last_mctp = p_mctp;
    }

    /* Mark the last MCTP message */
    p_last_mctp->end_of_message = 1;

    /* Register a dummy receiver */
    return mctp_set_rx_all(p_mctpusb->p_mctp, test_defrag_mctplib_dummy_rx, NULL);
}

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

char *test_defrag_mctplib_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Defragmentation test using 'libmctp'.";
    }
    else
    {
        return "In this test 25 MCTP packets are fragmented and sent through a dummy USB bus.\n"
               "The test ensures the correct reassembly of these fragments into complete messages which are then\n"
               "processed by a predefined receiver function.\n";
    }
}

/**
 * @brief Initializes the MCTP fragmentation test using libmctp.
 *
 * This function initializes the MCTP over USB transport layer by allocating 
 * necessary resources and initializing components.
  
  @return 0 on sucsess, else 1.
 */

int test_defrag_mctplib_init(uintptr_t arg)
{
    int ret;

    if ( p_mctpusb != NULL )
        return 1;

    /* Request RAM for this module, assert on failure */
    p_mctpusb = hal_alloc(sizeof(test_defrag_mctplib_session));
    if ( p_mctpusb == NULL )
        return 0;

    /* 
     * Create message queue pool to be used with libmctp. Later, we can use 
     * msgq_request() and msgq_release() to manage buffers in the free-busy list 
     * managed by the 'msgq' module.
     */

    /* Pool for MCTP packets */
    p_mctpusb->msgq_handle = msgq_create(MCTP_USB_MSGQ_MAX_FRAME_SIZE, MCTP_USB_MSGQ_ALLOCATED_FRAMES);
    if ( p_mctpusb->msgq_handle == 0 )
        ;
    return 0;

    /* Pool for MCTP context buffers */
    p_mctpusb->msgq_contex_handle = msgq_create(MCTP_USB_MAX_CONTEXT_SIZE, MCTP_USB_MSGQ_ALLOCATED_CONTEXES);
    if ( p_mctpusb->msgq_contex_handle == 0 )
        ;
    return 0;

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
    return ret;
}
