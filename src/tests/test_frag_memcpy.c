
/**
  ******************************************************************************
  *
  * @file test_frag_memcpy.c
  * @brief Exploding an NCSI ethetnet packet to mutiple frames using memcpy().
  *  
  * @note 
  *  
  * ******************************************************************************
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
#include <ncsi_packet.h>
#include <string.h>
#include <stdint.h>

#define USB_FRAME_SIZE   64 /* USB base packet size */
#define MCTP_HEADER_SIZE 4

/* Maximum size in byts for a single fragment */
#define MCTP_MAX_FRAGMNET_SIZE (USB_FRAME_SIZE + MCTP_HEADER_SIZE)

/* Max NC-SI Ethernet frames chunks, each of which upto 68 bytes in size */
#define MCTP_MAX_FRAGMENTS 25

/*
 * Generic pointer / length structutre.
 */

typedef struct ptr_size_pair_t
{
    uintptr_t ptr;  /* Pointer stored as an unsigned integer type. */
    size_t    size; /* Size associated with the pointer. */

} ptr_size_pair;

/* MCTP standard header */
typedef struct __attribute__((packed)) mctp_packet_t
{
    uint8_t version;              /* MCTP version */
    uint8_t destination_eid;      /* Destination Endpoint ID */
    uint8_t source_eid;           /* Source Endpoint ID */
    uint8_t message_tag      : 3; /* Message Tag */
    uint8_t tag_owner        : 1; /* Tag Owner */
    uint8_t packet_sequence  : 2; /* Packet Sequence Number */
    uint8_t end_of_message   : 1; /* End of Message Indicator */
    uint8_t start_of_message : 1; /* Start of Message Indicator */
    /* Integrity_check and reserved fields are omitted */

} mctp_ptr_packet;

/* 
 * MCTP and additional pointers packet structure
 * The MCTP header is fixed at 4 bytes (32 bits), so our additional pointers 
 * placed after the header at the end should not pose a problem.
 */

typedef struct mctp_frag_t
{

    mctp_ptr_packet     mctp_header;                     /* MCTP 4 bytes header */
    uint8_t             payload[MCTP_MAX_FRAGMNET_SIZE]; /* Pointer to the NC-SI packet */
    size_t              payload_size;                    /* Length of the payload data pointed to */
    struct mctp_frag_t *next;                            /* Pointer to the next packet */

} mctp_frag;

/**
 *
 * @brief Session structure holding persistent variables for the Frag test.
 *
 * This structure maintains the state and persistent variables used during 
 * the Frag test session. It stores pointers to the NC-SI Ethernet packet 
 * and the MCTP fragment list, along with MCTP versioning information and 
 * the current size and count of fragments.
 */

typedef struct frag_memcpy_test_t
{
    ncsi_eth_packet *p_ncsi_packet;    /**< Pointer to the currently handled  NC-SI Ethernet packet. */
    mctp_frag *      p_mctp_head;      /**< Head pointer to the MCTP fragments list. */
    uint8_t          ncsi_frgas_count; /**< Count of MCTP fragments used to  encapsulate the NC-SI packet. */
    uint16_t         ncsi_packet_size; /**< Size of the inbound NC-SI packet in bytes. */
    uint8_t          version;          /**< MCTP header version. */
    uint8_t          destination_eid;  /**< MCTP destination EID. */
    uint8_t          source_eid;       /**< MCTP source EID. */
    uint8_t          usb_tx_frames;
    uint8_t          usb_tx_operations;
    uint16_t         usb_raw_payload;

} frag_memcpy_test;

frag_memcpy_test *p_frag_memcpy_test = NULL;

/**
 * @brief Adjusts the pointers in the fragment list to correspond to a new 
 *        NC-SI packet.
 *
 * This function is called whenever a new NC-SI packet is received. It updates
 * the fragment list to point to the appropriate sections of the NC-SI packet
 * and adjusts the payload sizes accordingly. The last fragment in the list is 
 * marked with the end-of-message (EOM) flag.
 */

static void test_frag_memcpy_prep_fragments(void)
{
    mctp_frag *frag            = p_frag_memcpy_test->p_mctp_head;
    uint8_t *  p_payload       = (uint8_t *) p_frag_memcpy_test->p_ncsi_packet;
    size_t     remaining_bytes = p_frag_memcpy_test->ncsi_packet_size;

    /* Adjust the fragments list to the inbound NC-SI packet */
    for ( uint16_t i = 0; i < p_frag_memcpy_test->ncsi_frgas_count && frag; i++ )
    {
        /* Copy data and build the outgoing USB packet */
        frag->payload_size = (remaining_bytes > USB_FRAME_SIZE) ? USB_FRAME_SIZE : remaining_bytes;
        memcpy(frag->payload, p_payload, frag->payload_size);

        /* Update remaining bytes and payload pointer */
        p_payload += frag->payload_size;
        remaining_bytes -= frag->payload_size;

        /* Check if this is the last fragment */
        frag->mctp_header.end_of_message = (remaining_bytes == 0) ? 1 : 0;

        /* Move to the next fragment */
        frag = frag->next;
    }
}

/**
 * @brief Dummy function to simulate USB transmission of pointer-size pairs.
 *
 * This function is a placeholder for the actual USB transmission function. It 
 * is intended to simulate the transmission of an array of pointer-size pairs 
 * to the USB hardware.
 *
 * @param pairs       Array of pointer-size pairs representing data to be 
 *                    transmitted.
 * @param pairs_count The number of pairs in the array.
 */

static void test_frag_memcpy_on_usb_tx(ptr_size_pair *pairs, size_t pairs_count)
{
/* Implementation of USB transmission would go here. */
#ifdef DEBUG

    printf("\tUSB Tx %2d, messages: %d\n", p_frag_memcpy_test->usb_tx_operations, pairs_count);

    for ( int i = 0; i < pairs_count; i++ )
    {
        //printf("\tUSB TX [%2d]: %d bytres.\n",  p_frag_memcpy_test->usb_tx_frames, pairs[i].size);
        p_frag_memcpy_test->usb_tx_frames++;
        p_frag_memcpy_test->usb_raw_payload += pairs[i].size;
    }

    p_frag_memcpy_test->usb_tx_operations++;

#endif
}

/**
 * @brief Obtain a fake Ethernet NC-SI frame along with its size.
 *
 * In a real-world scenario, this frame would be placed in a designated RAM 
 * region, and this logic would be notified via an interrupt.
 * @note TBD: Confirm if this is the correct approach.
 *
 * @return 0 on success, 1 on failure.
 */

int test_frag_memcpy_prolog(uintptr_t arg)
{

    if ( p_frag_memcpy_test == NULL )
        return 1; /* Module not initalized */

    p_frag_memcpy_test->p_ncsi_packet = ncsi_request_packet(&p_frag_memcpy_test->ncsi_packet_size);
    if ( p_frag_memcpy_test->p_ncsi_packet == NULL || p_frag_memcpy_test->ncsi_packet_size == 0 )
        return 1;

    /* Prepended '3'to NCSI packet */
    p_frag_memcpy_test->p_ncsi_packet->extra_byte[3] = 3;
    p_frag_memcpy_test->usb_tx_frames                = 0;
    p_frag_memcpy_test->usb_raw_payload              = 0;
    p_frag_memcpy_test->usb_tx_operations            = 0;

    /* Calculate the total number of fragments needed */
    p_frag_memcpy_test->ncsi_frgas_count = (p_frag_memcpy_test->ncsi_packet_size + USB_FRAME_SIZE - 1) / USB_FRAME_SIZE;

    if ( p_frag_memcpy_test->ncsi_frgas_count > MCTP_MAX_FRAGMENTS )
    {
        /* Drop the packet , it's too big */
        p_frag_memcpy_test->ncsi_frgas_count = 0;
        return 1;
    }

#ifdef DEBUG
    printf("\tNC-SI inbound packet size: %d\n", p_frag_memcpy_test->ncsi_packet_size);
    printf("\tNC-SI expected fragments of up-to 64 bytes: %d\n", p_frag_memcpy_test->ncsi_frgas_count);
#endif

    return 0; /* NC-SI packet reday for frgmantation */
}

/**
 * @brief Performs the fragments test by processing and transmitting NC-SI 
 *        packet fragments.
 *
 * This function is executed when a new NC-SI packet is available. It adjusts 
 * the fragment pointers using `test_frag_memcpy_adjust_pointers()`, and then iterates 
 * through the fragments, preparing batches of MCTP headers and their 
 * corresponding payloads. These batches are then sent to the USB hardware 
 * in groups of up to 4 fragments (2 pairs of header and payload).
 *
 * @param arg Unused parameter, reserved for future use.
 */

void test_exec_memcpy_frag(uintptr_t arg)
{
    mctp_frag *frag        = p_frag_memcpy_test->p_mctp_head;
    size_t     pairs_count = 0;
    size_t     usb_tx_io   = 0;

    /* By now, we trust that there is a pending NC-SI packet */
    test_frag_memcpy_prep_fragments();

    /* Define an array to hold up to 4 pairs at a time */
    ptr_size_pair pairs[4];

    /* Iterate over the fragments and send them in batches of upto 4 */
    frag = p_frag_memcpy_test->p_mctp_head;
    for ( int i = 0; i < p_frag_memcpy_test->ncsi_frgas_count; i++ )
    {
        /* Create pair of packet and size */
        pairs[pairs_count].ptr  = (uintptr_t) &frag->mctp_header;
        pairs[pairs_count].size = sizeof(frag->mctp_header) + frag->payload_size;
        pairs_count++;

        /* Move to the next fragment */
        frag = frag->next;

        /* Check if we've collected 4 fragments (2 pairs) or reached 
           the last fragment */
        if ( pairs_count == 4 || i == p_frag_memcpy_test->ncsi_frgas_count - 1 )
        {
            /* Send the batch to the fake USB hardware interfcae */
            test_frag_memcpy_on_usb_tx(pairs, pairs_count);
            usb_tx_io++;
            pairs_count = 0;
        }
    }

/* Packet sent. */
#ifdef DEBUG
    printf("\n\tUSB fragments: %d\n", p_frag_memcpy_test->usb_tx_frames);
    printf("\tUSB IO: %d\n", usb_tx_io);
    printf("\tUSB TX bytes (with MCTP headers): %d\n\n", p_frag_memcpy_test->usb_raw_payload);

#endif
}

/**
 * @brief Provides a description for the NC-SI to MCTP packet fragmentation test.
 * 
 * This function returns a description of the test that handles NC-SI to MCTP packet 
 * fragmentation using a zero-copy method, either as a brief one-liner or as a more 
 * detailed explanation depending on the value of `description_type`.
 * 
 * @param description_type Specifies the type of description:
 *                         0 for a brief one-line description,
 *                         1 for an in-depth test description.
 * @return A pointer to a string containing the description.
 */

char *test_frag_memcpy_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "NC-SI to MCTP packet fragmentation flow using memcpy() method.";
    }
    else
    {
        return "This test simulates the reception and handling of an NC-SI packet, which is \n"
               "then fragmented into smaller chunks to fit within MCTP fragments, each \n"
               "attached to a 64-byte payload. The approach uses `memcpy()` to copy data \n"
               "into a contiguous buffer in RAM, allowing for efficient transmission of \n"
               "complete 68-byte packets to the USB peripheral.\n\n"
               "While this method involves additional CPU cycles due to the overhead of \n"
               "copying data, it significantly reduces USB I/O operations by consolidating \n"
               "the MCTP fragments into full USB frames. This reduces the number of smaller \n"
               "transmissions that would otherwise occur if the data were handled directly \n"
               "with pointer arithmetic.\n\n"
               "Finally, the contiguous buffer is passed to a simulated USB interface, which \n"
               "in a real-world application would transmit the data as a complete USB frame. \n"
               "This test aims to validate the efficiency and correctness of the fragmentation \n"
               "process using `memcpy()`, ensuring that the system can handle packet \n"
               "fragmentation and reassembly with minimal resource consumption and optimal \n"
               "performance, despite the increased CPU cycles required for memory operations.";
    }
}

/*
 * Initializes the module and allocates RAM for persistent variables.
 * 
 * Note: All fragments that we are allowed to use will be allocated during
 * initialization and populated with default values. Any allocation failure 
 * will result in an assertion.
 */

int test_frag_memcpy_init(uintptr_t arg)
{
    mctp_frag *frag      = NULL;
    int        msg_index = 0;

    if ( p_frag_memcpy_test != NULL )
        return 1; /* Must initialize only once */

    /* 
     * The following operations are performed once and do not count as 
     * 'test-related cycles'. 
     */

    p_frag_memcpy_test = hal_alloc(sizeof(frag_memcpy_test));
    assert(p_frag_memcpy_test != NULL);

    /* Sets MCTP defaults */
    p_frag_memcpy_test->version         = 1;
    p_frag_memcpy_test->destination_eid = 0x10;
    p_frag_memcpy_test->source_eid      = 0x20;
    p_frag_memcpy_test->p_mctp_head     = NULL;

    /* 
     * Allocate all MCTP fragments and attch them to the session head pointer.
     * For now, feel as many fealds as posiiable to reduice setup clocks 
     * at run-time. */

    while ( msg_index < MCTP_MAX_FRAGMENTS )
    {

        frag = hal_alloc(sizeof(mctp_frag));
        assert(frag != NULL);

        /* Initialize MCTP header */
        frag->mctp_header.version         = p_frag_memcpy_test->version;
        frag->mctp_header.destination_eid = p_frag_memcpy_test->destination_eid;
        frag->mctp_header.source_eid      = p_frag_memcpy_test->source_eid;
        frag->mctp_header.message_tag     = 0;
        frag->mctp_header.tag_owner       = 1;

        if ( msg_index == 0 )
        {
            frag->mctp_header.packet_sequence  = 0;
            frag->mctp_header.start_of_message = 1; /* Start of Message */
        }
        else
        {
            frag->mctp_header.packet_sequence++;
            frag->mctp_header.start_of_message = 0; /* Not Start of Message */
        }

        frag->mctp_header.end_of_message = 0; /* Not End of Message */

        /* Those are not set initially */
        frag->payload_size = 0;

        /* Attach to the galobal head pointert */
        LL_APPEND(p_frag_memcpy_test->p_mctp_head, frag);

        msg_index++;
    }

    /* We'rte ready. */
    return 0;
}
