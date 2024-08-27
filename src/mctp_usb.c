
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
#include <mctp_usb.h>
#include <libmctp-log.h>
#include <string.h>

#define SEQ(x) (x << MCTP_HDR_SEQ_SHIFT)

struct test
{
    char *  name;
    int     n_packets;
    uint8_t flags_seq_tags[4];
    int     exp_rx_count;
    size_t  exp_rx_len;
} tests[] = {
    {
        /* single packet */
        .name      = "Single packet",
        .n_packets = 1,
        .flags_seq_tags =
            {
                SEQ(1) | MCTP_HDR_FLAG_SOM | MCTP_HDR_FLAG_EOM,
            },
        .exp_rx_count = 1,
        .exp_rx_len   = 1,
    },
    {
        /* two packets: one start, one end */
        .name      = "Two packets: one start, one end",
        .n_packets = 2,
        .flags_seq_tags =
            {
                SEQ(1) | MCTP_HDR_FLAG_SOM,
                SEQ(2) | MCTP_HDR_FLAG_EOM,
            },
        .exp_rx_count = 1,
        .exp_rx_len   = 2,
    },
    {
        /* three packets: one start, one no flags, one end */
        .name      = "Three packets: one start, one no flags, one end",
        .n_packets = 3,
        .flags_seq_tags =
            {
                SEQ(1) | MCTP_HDR_FLAG_SOM,
                SEQ(2),
                SEQ(3) | MCTP_HDR_FLAG_EOM,
            },
        .exp_rx_count = 1,
        .exp_rx_len   = 3,
    },
    {
        /* two packets, wrapping sequence numbers */
        .name      = "Two packets, wrapping sequence numbers ",
        .n_packets = 2,
        .flags_seq_tags =
            {
                SEQ(3) | MCTP_HDR_FLAG_SOM,
                SEQ(0) | MCTP_HDR_FLAG_EOM,
            },
        .exp_rx_count = 1,
        .exp_rx_len   = 2,
    },
    {
        /* two packets, invalid sequence number */
        .name      = "Two packets, invalid sequence number",
        .n_packets = 2,
        .flags_seq_tags =
            {
                SEQ(1) | MCTP_HDR_FLAG_SOM,
                SEQ(3) | MCTP_HDR_FLAG_EOM,
            },
        .exp_rx_count = 0,
        .exp_rx_len   = 0,
    },
};

/**
 * @brief Holds all global variables for the module.
 */
typedef struct mctp_usb_session_t
{
    struct mctp *       p_mctp;      /*!< Pointer to the libmctp instance */
    struct mctp_binding binding;     /* LibMCTP binding container */
    mctp_eid_t          eid;         /*Our Endpoint ID */
    uintptr_t           msgq_handle; /*!< Handle to the message queue */

} mctp_usb_session;

/* Pointer to the module's session instance */
mctp_usb_session *p_mctpusb = NULL;

static int mctp_usbt_tx(struct mctp_binding *binding __attribute__((unused)), struct mctp_pktbuf *pkt __attribute__((unused)))
{
    /* Currently not expecting TX packets */
    return 0;
}

/**
 * @return
 */

void mctp_usb_rx_raw(struct mctp_binding *binding, void *buf, size_t len)
{
    struct mctp_pktbuf *pkt;

    pkt = mctp_pktbuf_alloc(binding, len);
    assert(pkt);

    hal_memcpy(mctp_pktbuf_hdr(pkt), buf, len);
    mctp_bus_rx(binding, pkt);
}

/**
 * @brief Retrieves the handle to the message queue initialized by this module.
 * @return The message queue handle, or 0 if the module or message queue is not 
 *         initialized.
 */

uintptr_t mctp_usb_get_msgq_handle(void)
{
#if ( HAL_PTR_SANITY_CHECKS > 0 )
    if ( p_mctpusb != NULL )
        return p_mctpusb->msgq_handle;

    return 0;
#else
    return p_mctpusb->msgq_handle;
#endif
}

static void mctp_usb_test_rx(uint8_t eid, bool tag_owner, uint8_t msg_tag, void *data, void *msg, size_t len)
{
}

static void mctp_usb_run_seq_cycles(uintptr_t ptr)
{
    const mctp_eid_t remote_eid = 10;
    struct test *    test       = (struct test *) ptr;

    struct
    {
        struct mctp_hdr hdr;
        uint8_t         payload[1];
    } pktbuf;

    for ( int i = 0; i < test->n_packets; i++ )
    {
        memset(&pktbuf, 0, sizeof(pktbuf));
        pktbuf.hdr.dest          = p_mctpusb->eid;
        pktbuf.hdr.src           = remote_eid;
        pktbuf.hdr.flags_seq_tag = test->flags_seq_tags[i];
        pktbuf.payload[0]        = i;

        mctp_usb_rx_raw(&p_mctpusb->binding, &pktbuf, sizeof(pktbuf));
    }
}

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

void mctp_usb_run_seq_tests(void)
{

    struct test *test   = NULL;
    uint64_t     cycles = 0;
    unsigned int test_num;

    mctp_set_rx_all(p_mctpusb->p_mctp, mctp_usb_test_rx, NULL);

    for ( test_num = 0; test_num < ARRAY_SIZE(tests); test_num++ )
    {
        test = &tests[test_num];

        printf("Running: %-64s: ", test->name);

        /* Execute and count cycles */
        cycles = hal_measure_cycles(mctp_usb_run_seq_cycles, (uintptr_t) test);

        printf("%6llu cycles\n", cycles);
    }
}

/**
 * @brief Initializes the MTCP over USB libmctp bus.
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

void mctp_usb_init(uint8_t eid)
{
    int ret;

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

    p_mctpusb->eid = eid;
    mctp_set_max_message_size(p_mctpusb->p_mctp, MCTP_USB_MSGQ_MAX_FRAME_SIZE);

    /* Binding */
    p_mctpusb->binding.name        = "USB";
    p_mctpusb->binding.version     = 1;
    p_mctpusb->binding.tx          = mctp_usbt_tx;
    p_mctpusb->binding.pkt_size    = MCTP_PACKET_SIZE(MCTP_BTU);
    p_mctpusb->binding.pkt_header  = 0;
    p_mctpusb->binding.pkt_trailer = 0;

    // xos_disable_interrupts();
    ret = mctp_register_bus(p_mctpusb->p_mctp, &p_mctpusb->binding, eid);
    assert(ret == 0);
}
