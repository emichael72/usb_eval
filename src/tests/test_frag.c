
/**
  ******************************************************************************
  *
  * @file test_frag.c
  * @brief Exploding an NCSI ethetnet packet to mutiple frames using zerocopy.
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
#include <ncsi.h>
#include <test_frag,h>
#include <tests.h>
#include <string.h>
#include <stdint.h>

#define MCTP_HEADER_SIZE             4
#define NCSI_MAX_FRAGMNET_SIZE       64
#define MCTP_MAX_FIRST_FRAGMNET_SIZE (NCSI_MAX_FRAGMNET_SIZE - 1)

/* Maximum size of all data chunks pointed to by the pointers set on the USB peripheral,
 * beyond which we risk MCTP packet fragmentation. */

#define USB_MAX_PAYLOAD_SIZE 512
#define USB_MAX_POINTERS     16

/* Maximum size in byts for a single MCTP fragment : header + payload  */
#define MCTP_MAX_FRAGMNET_SIZE (MCTP_HEADER_SIZE + NCSI_MAX_FRAGMNET_SIZE)

/* Max NC-SI Ethernet frames chunks, each of which upto 68 bytes in size */
#define MCTP_MAX_FRAGMENTS 25

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

} mctp_packet;

/* 
 * MCTP and additional pointers packet structure
 * The MCTP header is fixed at 4 bytes (32 bits), so our additional pointers 
 * placed after the header at the end should not pose a problem.
 */

typedef struct mctp_frag_t
{

    mctp_packet         mctp_header;  /* MCTP 4 bytes header */
    uint8_t *           payload;      /* Pointer to the NC-SI packet */
    size_t              payload_size; /* Length of the payload data pointed to */
    struct mctp_frag_t *next;         /* Pointer to the next packet */

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

typedef struct frag_test_t
{
    ncsi_eth_packet *p_ncsi_packet;             /**< Pointer to the current NC-SI Ethernet packet. */
    mctp_frag *      p_mctp_head;               /**< Head of the MCTP fragments list. */
    uint8_t *        p_ncsi_start;              /**< Pointer to the start of the NC-SI message. */
    cb_on_usb_tx     usb_tx_cb;                 /**< User defined - TX callkback */
    ptr_size_pair    pairs[USB_MAX_POINTERS];   /**< Array of pointers for USB operations. */
    size_t           ncsi_packet_size;          /**< Size of the NC-SI packet in bytes. */
    size_t           req_packet_size;           /**< Externaly set NC-SI packet size. */
    uint8_t          version;                   /**< MCTP header version. */
    uint8_t          destination_eid;           /**< MCTP destination EID. */
    uint8_t          source_eid;                /**< MCTP source EID. */
    uint8_t          ncsi_expected_frags_count; /**< Number of MCTP fragments for the NC-SI packet. */
    uint8_t          usb_tx_total_pointers;     /**< Total pointers used in all USB transmissions. */
    uint8_t          usb_tx_total_operations;   /**< Total USB transmission operations. */
    uint16_t         usb_tx_operation_pointers; /**< Pointers used in the current USB operation. */
    uint16_t         expected_tx_size;          /**< Total size in bytes for transmission. */
    uint16_t         usb_tx_operation_bytes;    /**< Bytes transmitted in the current USB operation. */
    uint16_t         usb_raw_payload;           /**< Total raw payload size in bytes, excluding MCTP headers. */

} frag_test;

frag_test *p_frag_test = NULL;

/**
 * @brief Calculates the expected number of MCTP fragments and total transmission size
 *        for the current NC-SI packet.
 *
 * This function determines how the NC-SI packet will be divided into MCTP fragments,
 * calculates the number of fragments required, and the total size of the data to be
 * transmitted, including MCTP headers. It checks if the packet exceeds the maximum
 * number of allowed fragments and returns an error if it does.
 *
 * @return int 0 on success, 1 if the packet is invalid or too large to process.
 */

static int test_frag_calculate_ncsi_fragments(void)
{
    size_t remaining_size;

    /* Start with the first fragment */
    p_frag_test->ncsi_expected_frags_count = 1;

    if ( p_frag_test->ncsi_packet_size > 63 )
    {
        remaining_size = p_frag_test->ncsi_packet_size - 63;
        p_frag_test->ncsi_expected_frags_count += (remaining_size + (NCSI_MAX_FRAGMNET_SIZE - 1)) / NCSI_MAX_FRAGMNET_SIZE;
    }

    if ( p_frag_test->ncsi_expected_frags_count > MCTP_MAX_FRAGMENTS )
    {
        /* Drop the packet, it's too big */
        p_frag_test->ncsi_expected_frags_count = 0;
#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
        printf("\n\tError: NC-SI packet size results in too many fragments.\n");
#endif
        return 1;
    }

#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
    /* Calculate the total number of bytes to transmit */
    p_frag_test->expected_tx_size = 0;

    /* Size of the first fragment (63 bytes payload + 4 bytes header) */
    p_frag_test->expected_tx_size += 63 + MCTP_HEADER_SIZE;

    if ( p_frag_test->ncsi_packet_size > 63 )
    {
        size_t remaining_size = p_frag_test->ncsi_packet_size - 63;

        /* Number of full fragments (each with 64 bytes payload + 4 bytes header) */
        size_t full_fragments = remaining_size / 64;
        p_frag_test->expected_tx_size += full_fragments * (64 + MCTP_HEADER_SIZE);

        /* Size of the last fragment, which may be less than 64 bytes */
        size_t last_fragment_size = remaining_size % 64;
        if ( last_fragment_size > 0 )
        {
            p_frag_test->expected_tx_size += last_fragment_size + MCTP_HEADER_SIZE;
        }
    }

#endif
    return 0; /* Success */
}

/**
 * @brief Adjusts the pointers in the fragment list to correspond to a new 
 *        NC-SI packet.
 *
 * This function is called whenever a new NC-SI packet is received. It updates
 * the fragment list to point to the appropriate sections of the NC-SI packet
 * and adjusts the payload sizes accordingly. The last fragment in the list is 
 * marked with the end-of-message (EOM) flag.
 */

static void test_frag_adjust_pointers(void)
{
    mctp_frag *frag            = p_frag_test->p_mctp_head;
    uint8_t *  p_payload       = p_frag_test->p_ncsi_start;
    size_t     remaining_bytes = p_frag_test->ncsi_packet_size;

    /* Ensure the first fragment does not exceed 63 bytes */
    size_t first_packet_size = (remaining_bytes > MCTP_MAX_FIRST_FRAGMNET_SIZE) ? MCTP_MAX_FIRST_FRAGMNET_SIZE : remaining_bytes;

    /* Adjust the fragments list to the inbound NC-SI packet */
    int is_first_fragment = 1; /* Track if this is the first fragment */

    while ( frag != NULL && remaining_bytes > 0 )
    {
        /* Set the payload pointer */
        frag->payload = p_payload;

        /* Set the payload size */
        if ( is_first_fragment )
        {
            frag->payload_size = first_packet_size;
            is_first_fragment  = 0; /* Subsequent fragments will use the regular size */
        }
        else
        {
            frag->payload_size = (remaining_bytes > NCSI_MAX_FRAGMNET_SIZE) ? NCSI_MAX_FRAGMNET_SIZE : remaining_bytes;
        }

        /* Update the remaining bytes and the payload pointer */
        p_payload += frag->payload_size;
        remaining_bytes -= frag->payload_size;

        /* Check if this is the last fragment and set the EOM flag */
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

static void test_frag_on_usb_tx(ptr_size_pair *pairs, size_t pairs_count)
{
/* Implementation of USB transmission would go here. */
#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)

    printf("\n\tUSB op. # %u:\n", p_frag_test->usb_tx_total_operations);

    for ( int i = 0; i < pairs_count; i++ )
    {
        printf("\n");
        hal_hexdump((void *) pairs[i].ptr, pairs[i].size, false, "\t");
        p_frag_test->usb_raw_payload += pairs[i].size;
    }
    printf("\n");

#endif

    /* Jump to user TX handler when specified */
    if ( p_frag_test->usb_tx_cb != NULL )
        p_frag_test->usb_tx_cb(pairs, pairs_count);
}

/**
 * @brief Perform final validation after the test measurement is completed.
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 *
 * @return 0 if the validation process completes successfully.
 */

int test_frag_epilog(uintptr_t arg)
{

    mctp_frag *frag;
    uint8_t    seq = 0;

    LL_FOREACH(p_frag_test->p_mctp_head, frag)
    {

        /* Initialize MCTP header */
        frag->mctp_header.version         = p_frag_test->version;
        frag->mctp_header.destination_eid = p_frag_test->destination_eid;
        frag->mctp_header.source_eid      = p_frag_test->source_eid;
        frag->mctp_header.end_of_message  = 0; /* Not End of Message */
        frag->mctp_header.message_tag     = 0;
        frag->mctp_header.tag_owner       = 1;
        frag->mctp_header.packet_sequence = seq;

        if ( seq == 0 )
            frag->mctp_header.start_of_message = 1; /* Start of Message */
        else
            frag->mctp_header.start_of_message = 0; /* Not Start of Message */

        /* Those are not set initially */
        frag->payload      = NULL;
        frag->payload_size = 0;
        seq++;
    }

    memset(&p_frag_test->pairs, 0, sizeof(p_frag_test->pairs));

    if ( p_frag_test->p_ncsi_packet != NULL )
        ncsi_release_packet(p_frag_test->p_ncsi_packet);

    p_frag_test->p_ncsi_packet             = NULL;
    p_frag_test->p_ncsi_start              = NULL;
    p_frag_test->ncsi_packet_size          = 0;
    p_frag_test->usb_raw_payload           = 0;
    p_frag_test->usb_tx_total_pointers     = 0;
    p_frag_test->usb_tx_total_operations   = 0;
    p_frag_test->usb_tx_operation_bytes    = 0;
    p_frag_test->usb_tx_operation_pointers = 0;

    return 0;
}

/**
 * @brief Performs the fragments test by processing and transmitting NC-SI 
 *        packet fragments.
 * 
 * <  Cycles mesurmett entry point>
 * 
 * This function is executed when a new NC-SI packet is available. It adjusts 
 * the fragment pointers using `test_frag_adjust_pointers()`, and then iterates 
 * through the fragments, preparing batches of MCTP headers and their 
 * corresponding payloads. These batches are sent to the USB hardware while
 * packing as many pairs as possible, ensuring that the total size does not 
 * exceed `MCTP_MAX_FRAGMENT_PAYLOAD_SIZE`. Each MCTP header must be transmitted 
 * with its corresponding payload in the same USB batch.
 *
 * @param arg Unused parameter, reserved for future use.
 */

void test_exec_frag(uintptr_t arg)
{
    mctp_frag *frag        = p_frag_test->p_mctp_head;
    size_t     pairs_count = 0;
    size_t     header_size = sizeof(frag->mctp_header);

    /* By now, we trust that there is a pending NC-SI packet */
    test_frag_adjust_pointers();

    /* Iterate over the fragments and send them in batches */
    while ( frag != NULL && frag->payload_size > 0 )
    {
        size_t current_pair_size = header_size + frag->payload_size;

        /* Check if adding the current fragment would exceed the max payload size */
        if ( (p_frag_test->usb_tx_operation_bytes + current_pair_size > USB_MAX_PAYLOAD_SIZE) || (pairs_count + 2 > USB_MAX_POINTERS) )
        {
            /* Send the current batch to the USB hardware */
            test_frag_on_usb_tx(p_frag_test->pairs, pairs_count);

            /* Reset counters */
            pairs_count                         = 0;
            p_frag_test->usb_tx_operation_bytes = 0;

#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
            printf("\n");
            p_frag_test->usb_tx_total_operations++;
            p_frag_test->usb_tx_operation_pointers = 0;
#endif
        }

        /* Add the MCTP header and its size */
        p_frag_test->pairs[pairs_count].ptr  = (uintptr_t) &frag->mctp_header;
        p_frag_test->pairs[pairs_count].size = header_size;
        pairs_count++;

        /* Add the payload pointer and its size */
        p_frag_test->pairs[pairs_count].ptr  = (uintptr_t) frag->payload;
        p_frag_test->pairs[pairs_count].size = frag->payload_size;
        pairs_count++;

        p_frag_test->usb_tx_operation_bytes += current_pair_size;

#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
        p_frag_test->usb_tx_total_pointers += 2;
        p_frag_test->usb_tx_operation_pointers += 2;
        printf("\tUSB adding TX pointer: size: %-3u, pointers %-2u\n", p_frag_test->usb_tx_operation_bytes, p_frag_test->usb_tx_operation_pointers);
#endif

        /* Move to the next fragment */
        frag = frag->next;
    }

    /* If there's remaining data to send, send it now */
    if ( pairs_count > 0 )
    {

        test_frag_on_usb_tx(p_frag_test->pairs, pairs_count);
#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
        p_frag_test->usb_tx_total_operations++;
        printf("\n");
#endif
    }

#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
    printf("\n\n\tUSB total pointers: %d\n", p_frag_test->usb_tx_total_pointers);
    printf("\tUSB total TX operations: %d\n", p_frag_test->usb_tx_total_operations);
    printf("\tUSB total TX bytes: %d\n\n", p_frag_test->usb_raw_payload);
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

int test_frag_prologue(uintptr_t arg)
{

    if ( p_frag_test == NULL )
        return 1; /* Module not initalized */

    if ( arg != 0 )
        p_frag_test->ncsi_packet_size = (size_t) arg;
    else
        p_frag_test->ncsi_packet_size = p_frag_test->req_packet_size;

    p_frag_test->p_ncsi_packet = ncsi_request_packet(&p_frag_test->ncsi_packet_size);
    if ( p_frag_test->p_ncsi_packet == NULL || p_frag_test->ncsi_packet_size == 0 )
        return 1;

    /* Prepended '3'to NCSI packet */

    /* According to architecture, we must prepend an extra byte to the first
        message payload and set it to 3. In this implementation, we've added 32 bits
        (4 bytes) to the NC-SI packet to maintain alignment. Now, we are setting the
        last byte of this 32-bit segment to 3. This adjustment ensures that the first
        message in the packet starts with the value '3', but it does so at the expense
        of the first MCTP message not beginning at an aligned address.
        
        Architectural note: While we ensured that the NC-SI packet remains aligned,
        the first MCTP message will start at a non-aligned address due to this 
        adjustment.
    */

    p_frag_test->p_ncsi_packet->extra_byte[3] = 3;
    p_frag_test->p_ncsi_start                 = (uint8_t *) (&(p_frag_test->p_ncsi_packet->extra_byte[3]));
    p_frag_test->ncsi_packet_size -= 3; /* We've added 32 bits to the NC-SI and we need only the last byte */
    p_frag_test->usb_raw_payload           = 0;
    p_frag_test->usb_tx_total_pointers     = 0;
    p_frag_test->usb_tx_total_operations   = 0;
    p_frag_test->usb_tx_operation_bytes    = 0;
    p_frag_test->usb_tx_operation_pointers = 0;

    /* Calculate expected fragment count and projected transmission size in bytes */
    if ( test_frag_calculate_ncsi_fragments() != 0 )
    {
        /* Drop the packet , it's too big */
        p_frag_test->ncsi_expected_frags_count = 0;
#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
        printf("\n\tError: NC-`SI packet size results in too many fragments.\n");
#endif
        return 1;
    }

#if defined(DEBUG) && (TEST_CONTINUOUS_MODE == 0)
    printf("\n\tNC-SI inbound packet size: %d\n", p_frag_test->ncsi_packet_size);
    printf("\tNC-SI expected fragments of up-to %d bytes: %d\n", MCTP_MAX_FRAGMNET_SIZE, p_frag_test->ncsi_expected_frags_count);
    printf("\tExpected transmision: %u bytes.\n\n", p_frag_test->expected_tx_size);
#endif

    return 0; /* NC-SI packet reday for frgmantation */
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

char *test_frag_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "'frag' using zero-copy method.";
    }
    else
    {
        return "This test simulates the reception and handling of an NC-SI packet, which is \n"
               "then fragmented into smaller chunks to fit within MCTP fragments, each \n"
               "attached to a 64-byte payload. The approach leverages zero-copy techniques \n"
               "by using pointer arithmetic to avoid the overhead associated with resource-\n"
               "intensive functions like memcpy().\n\n"
               "Instead of copying data, pointers are created to directly reference segments \n"
               "of the inbound NC-SI payload. These pointers form a list that points to the \n"
               "different chunks of the original Ethernet packet, allowing the system to \n"
               "efficiently handle the fragmentation without duplicating data in memory.\n\n"
               "Finally, these pointers are passed to a simulated USB interface, which, in a \n"
               "real-world application, would take these MCTP fragments, consolidate them \n"
               "into contiguous USB frames, and transmit them. This test aims to validate the \n"
               "efficiency and correctness of the fragmentation process, ensuring that the \n"
               "system can handle packet fragmentation and reassembly with minimal resource \n"
               "consumption and optimal performance.";
    }
}

/*
 * Initializes the module and allocates RAM for persistent variables.
 * 
 * Note: All fragments that we are allowed to use will be allocated during
 * initialization and populated with default values. Any allocation failure 
 * will result in an assertion.
 */

int test_frag_init(uintptr_t arg)
{
    mctp_frag *frag      = NULL;
    uint8_t    seq       = 0;
    int        msg_index = 0;

    if ( p_frag_test != NULL )
    {
        return 0; /* Must initialize only once */
    }

    /* 
     * The following operations are performed once and do not count as 
     * 'test-related cycles'. 
     */

    p_frag_test = hal_alloc(sizeof(frag_test));
    if ( p_frag_test == NULL )
        return 1;

    /* Sets MCTP defaults */
    p_frag_test->version         = 1;
    p_frag_test->destination_eid = 0x10;
    p_frag_test->source_eid      = 0x20;
    p_frag_test->p_mctp_head     = NULL;
    p_frag_test->req_packet_size = NCSI_PACKET_MAX_SIZE;

    if ( arg != 0 )
        p_frag_test->usb_tx_cb = (cb_on_usb_tx) arg;

    /* 
     * Allocate all MCTP fragments and attch them to the session head pointer.
     * For now, feel as many fealds as posiiable to reduice setup clocks 
     * at run-time. */

    while ( msg_index < MCTP_MAX_FRAGMENTS )
    {

        frag = hal_alloc(sizeof(mctp_frag));
        if ( frag == NULL )
            return 1;

        /* Initialize MCTP header */
        frag->mctp_header.version         = p_frag_test->version;
        frag->mctp_header.destination_eid = p_frag_test->destination_eid;
        frag->mctp_header.source_eid      = p_frag_test->source_eid;
        frag->mctp_header.end_of_message  = 0; /* Not End of Message */
        frag->mctp_header.message_tag     = 0;
        frag->mctp_header.tag_owner       = 1;
        frag->mctp_header.packet_sequence = seq;

        if ( msg_index == 0 )
            frag->mctp_header.start_of_message = 1; /* Start of Message */
        else
            frag->mctp_header.start_of_message = 0; /* Not Start of Message */

        /* Those are not set initially */
        frag->payload      = NULL;
        frag->payload_size = 0;
        seq++;

        /* Attach to the galobal head pointert */
        LL_APPEND(p_frag_test->p_mctp_head, frag);

        msg_index++;
    }

    /* We'rte ready. */
    return 0;
}
