
/**
  ******************************************************************************
  * @file    test_mctplib.c
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
#include <hal_msgq.h>
#include <test_mctplib.h>
#include <libmctp-log.h>
#include <string.h>

#define SEQ(x) (x << MCTP_HDR_SEQ_SHIFT)

/**
 * @brief The `test_mctplib_seq` structure is used to represent a sequence of 
 *        tests for the MCTP library. It encapsulates information about each test case, 
 *        including a description, the number of packets, expected outcomes, and relevant 
 *        flags or sequence tags.
 */

typedef struct test_mctplib_seq_t
{
    char    name[64];           /**< Pointer to a string that describes the test case. */
    int     n_packets;          /**< Number of packets involved in the test case. */
    uint8_t flags_seq_tags[32]; /**< Array of 4 bytes representing flags and sequence
                                    tags for the packets. Each byte controls the
                                    behavior of the corresponding packet in the test.*/
} test_mctplib_seq;

/* clang-format off */
test_mctplib_seq seq_tests[] = {

/*    Test description                         | n_packets | flags_seq_tags                                                              */
/*  -------------------------------------------------------------------------------------------------------------------------------------*/
 //   { "Single packet",                                   1, { SEQ(1) | MCTP_HDR_FLAG_SOM | MCTP_HDR_FLAG_EOM }, },
 //   { "Two packets: one start, one end",                 2, { SEQ(1) | MCTP_HDR_FLAG_SOM, SEQ(2) | MCTP_HDR_FLAG_EOM }, },
    { "Six packets: one start, 4 no flags, one end", 6, { SEQ(1) | MCTP_HDR_FLAG_SOM, SEQ(2), SEQ(3), SEQ(4),SEQ(5), SEQ(6) | MCTP_HDR_FLAG_EOM },    },
  //  { "Two packets, wrapping sequence numbers",          2, { SEQ(3) | MCTP_HDR_FLAG_SOM, SEQ(0) | MCTP_HDR_FLAG_EOM }, },
  //  { "Two packets, invalid sequence number",            2, { SEQ(1) | MCTP_HDR_FLAG_SOM, SEQ(3) | MCTP_HDR_FLAG_EOM }, }
};
/* clang-format on */

/**
 * @brief Holds all global variables for the module.
 */
typedef struct test_mctplib_session_t
{
    struct mctp *       p_mctp;             /* Pointer to the libmctp instance */
    struct mctp_binding binding;            /* libmctp binding container */
    mctp_eid_t          eid;                /* Our Endpoint ID */
    uintptr_t           msgq_handle;        /* Handle to the message queue */
    uintptr_t           msgq_contex_handle; /* Handle to the message queue dedicated for context buffers */

} test_mctplib_session;

/* Pointer to the module's session instance */
test_mctplib_session *p_mctpusb = NULL;

/* Dummy implementation of a real 'USB Tx' interface */
static int test_mctplib_tx(struct mctp_binding *binding __attribute__((unused)), struct mctp_pktbuf *pkt __attribute__((unused)))
{
    /* Currently not expecting TX packets */
    return 0;
}

/**
 * Pushes raw MCTP data to the 'USB RX' bus.
 *
 * This function allocates a new packet buffer from the pool and copies the 
 * provided data into the buffer as an MCTP packet. The buffer is then handed 
 * over to the MCTP core RX driver for further processing.
 *
 * @param buf Pointer to the data buffer containing the MCTP packet.
 * @param len Length of the data buffer.
 */

void test_mctplib_rx(void *buf, size_t len)
{
    struct mctp_pktbuf *pkt;

    pkt = mctp_pktbuf_alloc(&p_mctpusb->binding, len);
    assert(pkt);

    memcpy(mctp_pktbuf_hdr(pkt), buf, len);

    xt_iss_profile_enable();
    mctp_bus_rx(&p_mctpusb->binding, pkt);
    xt_iss_profile_disable();
}

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t test_mctplib_get_handle(size_t type)
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

/* MCTP standard header */
typedef struct mctplib_header_t
{
    uint8_t version;              /* MCTP version */
    uint8_t dest;                 /* Destination Endpoint ID */
    uint8_t src;                  /* Source Endpoint ID */
    uint8_t message_tag      : 3; /* Message Tag */
    uint8_t tag_owner        : 1; /* Tag Owner */
    uint8_t packet_sequence  : 2; /* Packet Sequence Number */
    uint8_t end_of_message   : 1; /* End of Message Indicator */
    uint8_t start_of_message : 1; /* Start of Message Indicator */

} mctplib_header;

static void test_mctplib_dummy_rx(uint8_t eid, bool tag_owner, uint8_t msg_tag, void *data, void *msg, size_t len)
{
    /* Do not do anything here, the cycles are counting :) */
#ifdef DEBUG
    printf("Dummy RX receiver got message from %d, %d bytes.\n", eid, len);
#endif
}

#define FRAG_SIZE   64
#define FRAGS_COUNT 23

typedef struct test_pkt_t
{
    mctplib_header hdr;
    uint8_t        payload[FRAG_SIZE];
} test_pkt;

test_pkt         pktbuf     = {0};
const mctp_eid_t remote_eid = 10;

void test_exec_mctplib(uintptr_t ptr)
{

    xt_iss_profile_disable();

    for ( int i = 0; i < FRAGS_COUNT; i++ )
    {

        pktbuf.hdr.dest             = p_mctpusb->eid;
        pktbuf.hdr.src              = remote_eid;
        pktbuf.hdr.start_of_message = 0;
        pktbuf.hdr.end_of_message   = 0;
        pktbuf.payload[0]           = i;

        if ( i == 0 )
            pktbuf.hdr.start_of_message = 1;
        if ( i == (FRAGS_COUNT - 1) )
            pktbuf.hdr.end_of_message = 1;

        test_mctplib_rx(&pktbuf, sizeof(pktbuf));

        pktbuf.hdr.packet_sequence++;
    }
}

/* libmctp integration prolog  */
int test_mctplib_prolog(uintptr_t ptr)
{
    /* Register a dummy receiver */
    return mctp_set_rx_all(p_mctpusb->p_mctp, test_mctplib_dummy_rx, NULL);
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

char *test_mctplib_desc(size_t description_type)
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

int test_mctplib_init(uintptr_t eid)
{
    int ret;

    if ( p_mctpusb != NULL )
        return 1;

    /* Request RAM for this module, assert on failure */
    p_mctpusb = hal_alloc(sizeof(test_mctplib_session));
    assert(p_mctpusb != NULL);

    /* 
     * Create message queue pool to be used with libmctp. Later, we can use 
     * msgq_request() and msgq_release() to manage buffers in the free-busy list 
     * managed by the 'msgq' module.
     */

    p_mctpusb->msgq_handle = msgq_create(MCTP_USB_MSGQ_MAX_FRAME_SIZE, MCTP_USB_MSGQ_ALLOCATED_FRAMES);
    assert(p_mctpusb->msgq_handle != 0);

    p_mctpusb->msgq_contex_handle = msgq_create(MCTP_USB_MAX_CONTEXT_SIZE, MCTP_USB_MSGQ_ALLOCATED_CONTEXES);
    assert(p_mctpusb->msgq_contex_handle != 0);

    /* Initialize libmctp, assert on error. */
    p_mctpusb->p_mctp = mctp_init();
    assert(p_mctpusb->p_mctp != NULL);

    p_mctpusb->eid = (int) eid;
    mctp_set_max_message_size(p_mctpusb->p_mctp, MCTP_USB_MSGQ_MAX_FRAME_SIZE);

#ifdef DEBUG
    mctp_set_log_stdio(MCTP_LOG_DEBUG);
#endif

    /* Binding */
    p_mctpusb->binding.name        = "USB";
    p_mctpusb->binding.version     = 1;
    p_mctpusb->binding.tx          = test_mctplib_tx;
    p_mctpusb->binding.pkt_size    = MCTP_BODY_SIZE(MCTP_USB_MSGQ_MAX_FRAME_SIZE) - 16;
    p_mctpusb->binding.pkt_header  = 0;
    p_mctpusb->binding.pkt_trailer = 0;

    ret = mctp_register_bus(p_mctpusb->p_mctp, &p_mctpusb->binding, eid);
    assert(ret == 0);

    return ret;
}
