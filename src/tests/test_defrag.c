/**
  ******************************************************************************
  * @file    test_defrag_mctplib.c
  * @author  IMCv2 Team
  * @brief   Aggregates about 25 fragments of MCTP packets into a single NC-SI.
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
#include <ncsi.h>
#include <test_frag,h>
#include <test_defrag.h>
#include <tests.h>
#include <string.h>

#define DEFRAG_PERFORM_SEQ_VALIDATION       (1)
#define DEFRAG_PERFORM_FIRSTBYTE_VALIDATION (1)

/* Macro to check if a pointer is 1 byte offset from the nearest 
   4-byte aligned address (i.e., unaligned address) */
#define DEFRAG_CHECK_OPTIMIZED_OFFSET(ptr)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if ( ((uintptr_t) (ptr) &0x3) != 0x1 )                                                                         \
        {                                                                                                              \
            assert(! "Pointer is not 1 byte offset from the nearest aligned address. Please check the optimization."); \
        }                                                                                                              \
    } while ( 0 )

/* MCTP standard packet along with 64 bytes payload  */
typedef struct mctp_packet_t
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

} mctp_packet;

/* 
 * A single USB packet
 */

typedef struct _usb_packet_t
{
    struct _usb_packet_t *next; /* Pointer to the next packet */
    void *                data; /* Frame data */
    size_t                size; /* Length of the payload data stored in the packet */

} usb_packet;

/**
 * @brief Structure representing a defragmentation session.
 *
 * This structure holds all the necessary information and buffers required
 * to perform and manage the defragmentation of MCTP packets received over
 * USB. It maintains pointers to the fragmented packets, reconstructed NC-SI
 * packets, and relevant size and count metrics for effective processing and
 * validation.
 */

typedef struct test_defrag_session_t
{
    uint8_t *        p_ncsi_start;      /**< Pointer to the start of the NC-SI message buffer. */
    usb_packet *     p_usb_packets;     /**< Head pointer to the linked list of USB packet fragments. */
    ncsi_eth_packet *p_ncsi_packet;     /**< Pointer to the current NC-SI Ethernet packet. */
    char *           error;             /**< Simple method to set error message from within the tested function */
    size_t           ncsi_packet_size;  /**< Size of the complete NC-SI packet in bytes. */
    size_t           req_packet_size;   /**< Externaly set NC-SI packet size. */
    uint16_t         usb_packets_count; /**< Number of USB packet fragments received. */
    uint16_t         rx_raw_size;       /**< Total size of the received raw data after defragmentation. */
    uint16_t         usb_raw_size;      /**< Total size of the raw data received from USB packets. */
    uint16_t         usb_offset;        /**< Offset to track position in the buffer */

} test_defrag_session;

/* Pointer to the module's session instance */
test_defrag_session *p_defrag_test = NULL;

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

void test_defrag_on_usb_tx(ptr_size_pair *pairs, size_t pairs_count)
{
    /* Collect all fragments into long USB frames, each containing multiple 
     * MCTP frames. These frames will serve as the inputs for the 
     * fragmentation test.*/

    usb_packet *packet;
    size_t      total_size = 0;
    size_t      offset     = 0;

    if ( pairs == NULL || pairs_count == 0 )
        assert(0);

    /* Create 'USB' list item, ToDo: switch to message q*/
    packet = (usb_packet *) malloc(sizeof(usb_packet));
    assert(packet != NULL);

    /* Get the total size of all the data included with the pairs */
    for ( int i = 0; i < pairs_count; i++ ) total_size += pairs[i].size;

    p_defrag_test->usb_raw_size += total_size; /* Get the total size of all chunks */

    /* ToDo : Switch to fixed size message Q */
    packet->data = malloc(total_size);
    assert(packet->data != NULL);

    packet->size = total_size;

    /* Append each pairs->ptr to packet->data */
    for ( int i = 0; i < pairs_count; i++ )
    {
        memcpy((uint8_t *) packet->data + offset, (void *) pairs[i].ptr, pairs[i].size);
        offset += pairs[i].size;
    }

    /* Attach the aggregated packet to the packets list */
    LL_APPEND(p_defrag_test->p_usb_packets, packet);
}

/**
 * @brief Perform final validation after the test measurement is completed.
 *
 * This function validates the final size of the assembled packet and provides
 * feedback on the success or failure of the test. It is intended to be called
 * after the defragmentation process has finished.
 *
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 *
 * @return 0 if the validation process completes successfully.
 */

int test_defrag_epilog(uintptr_t arg)
{
    usb_packet *packet, *tmp;

/* Final size validation */
#if ( TEST_CONTINUOUS_MODE == 0 )
    if ( p_defrag_test->usb_offset != p_defrag_test->ncsi_packet_size )
    {
        printf("Error: The assembled packet size (%d) does not match the expected size (%d).\n", p_defrag_test->usb_offset, p_defrag_test->ncsi_packet_size);
        if ( p_defrag_test->error )
            printf("%s", p_defrag_test->error);
    }
    else
    {
        printf("Success: Assembled packet (%d total, %d payload) passed all tests.\n", p_defrag_test->ncsi_packet_size,
               NCSI_GET_PAYLOAD_CLEAN(p_defrag_test->ncsi_packet_size));
    }
#endif
    /* Reset state and pointers for being able to measure continuously*/
    if ( p_defrag_test->p_ncsi_packet != NULL )
        free(p_defrag_test->p_ncsi_packet);

    /* Free USB buffers - temporary solution for POC only. */
    LL_FOREACH_SAFE(p_defrag_test->p_usb_packets, packet, tmp)
    {
        if ( packet != NULL )
        {
            if ( packet->data != NULL )
                free(packet->data);

            /* List detach */
            LL_DELETE(p_defrag_test->p_usb_packets, packet);

            free(packet);
        }
    }

    p_defrag_test->req_packet_size   = NCSI_PACKET_MAX_SIZE;
    p_defrag_test->p_usb_packets     = NULL;
    p_defrag_test->p_ncsi_start      = NULL;
    p_defrag_test->usb_packets_count = 0;
    p_defrag_test->usb_raw_size      = 0;
    p_defrag_test->usb_offset        = 0;
    p_defrag_test->error             = NULL;
    p_defrag_test->ncsi_packet_size  = 0;

    /* Call the 'frga' epilog since we've been using it in ther background to
       generate all the fragments */

    return test_frag_epilog(0);
}

/**
 * @brief Defragments MCTP packets from USB chunks into a contiguous buffer.
 *
 * This function processes a series of USB packets, each containing MCTP packets.
 * It validates the MCTP sequence numbers and assembles the MCTP payloads into
 * a contiguous buffer. The first packet is treated specially, where a byte '3'
 * is skipped after the MCTP header.
 *  
 * 
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 */

void test_exec_defrag(uintptr_t arg)
{
    usb_packet *packet          = p_defrag_test->p_usb_packets; /* Point to head */
    uint8_t *   ncsi_packet_ptr = p_defrag_test->p_ncsi_start;

#if ( DEFRAG_PERFORM_SEQ_VALIDATION > 0 )
    uint8_t expected_sequence = 0; /* Expected MCTP sequence number */
#endif

    /* Process each packet */
    while ( packet != NULL )
    {
        size_t   packet_offset  = 0;
        size_t   remaining_size = packet->size;
        uint8_t *packet_data    = packet->data; /* Cache pointer to data to minimize memory access */

        while ( remaining_size > 0 )
        {
            mctp_packet *mctp        = (mctp_packet *) (packet_data + packet_offset);
            size_t       header_size = 4;
            size_t       payload_size;

#if ( DEFRAG_PERFORM_SEQ_VALIDATION > 0 )
            if ( mctp->packet_sequence != expected_sequence )
            {
                p_defrag_test->error = "Error: packet dropped, sequence number mismatch.\n";
                return;
            }
#endif

            /* Handle the first packet differently */
            if ( packet == p_defrag_test->p_usb_packets && packet_offset == 0 )
            {
#if ( DEFRAG_PERFORM_FIRSTBYTE_VALIDATION > 0 )
                /* Validate and skip the first byte after the MCTP header */
                if ( mctp->payload[0] != 3 )
                {
                    p_defrag_test->error = "Error: first byte after the MCTP header must be 3.\n";
                    return;
                }
#endif

                /* Calculate and copy the first packet's payload */
                payload_size = (mctp->end_of_message) ? (remaining_size - header_size) : 63;
                memcpy(ncsi_packet_ptr + p_defrag_test->usb_offset, mctp->payload, payload_size);

                /* Update offsets */
                p_defrag_test->usb_offset += payload_size;
                packet_offset += header_size + 63; /* 4 bytes header + 63 bytes payload */
                remaining_size -= header_size + 63;
            }
            else
            {
                /* Calculate the payload size for normal packets */
                payload_size = (mctp->end_of_message) ? (remaining_size - header_size) : 64;

                /* Copy the payload directly */
                memcpy(ncsi_packet_ptr + p_defrag_test->usb_offset, mctp->payload, payload_size);

                /* Update offsets */
                p_defrag_test->usb_offset += payload_size;
                packet_offset += header_size + payload_size;
                remaining_size -= header_size + payload_size;
            }

#if ( DEFRAG_PERFORM_SEQ_VALIDATION > 0 )
            /* Increment the expected sequence number and handle wrap-around */
            expected_sequence = (expected_sequence + 1) & 0x03;
#endif
        }

        /* Move to the next packet only after the current one is fully processed */
        packet = packet->next;
    }
}

/**
 * @brief Prepares the defragmentation module and generates MCTP fragments.
 *
 * This function initializes the defragmentation module, invokes the fragment
 * generation process, and counts the received frames. It also prepares the
 * necessary structures for defragmentation and ensures the module is ready
 * for further processing.
 *
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 *
 * @return 0 if the module is ready, 1 if initialization failed or no data was
 *         received.
 */

int test_defrag_prologue(uintptr_t arg)
{
    usb_packet *packet;

    if ( p_defrag_test == NULL )
        return 1; /* Module not initialized */

    p_defrag_test->usb_packets_count = 0;
    p_defrag_test->usb_raw_size      = 0;
    p_defrag_test->usb_offset        = 0;
    p_defrag_test->error             = NULL;
    p_defrag_test->ncsi_packet_size  = (size_t) arg;

    if ( p_defrag_test->ncsi_packet_size == 0 )
        p_defrag_test->ncsi_packet_size = p_defrag_test->req_packet_size;

    if ( p_defrag_test->ncsi_packet_size <= NCSI_HEADERS_SIZE || p_defrag_test->ncsi_packet_size > NCSI_PACKET_MAX_SIZE )
        return 1;

    /* Allocate buffer for the assembled packet */
    p_defrag_test->p_ncsi_packet = (ncsi_eth_packet *) malloc(p_defrag_test->ncsi_packet_size);
    if ( p_defrag_test->p_ncsi_packet == NULL )
        return 1;

    /* Optimization:
     * 
     * The following line is designed to optimize memory access for large NC-SI packets by 
     * utilizing Xtensa's ability to efficiently copy 16 bytes at a time, provided the 
     * source, destination, and length are aligned. */

    p_defrag_test->p_ncsi_start = (uint8_t *) (&(p_defrag_test->p_ncsi_packet->extra_byte[1]));
    DEFRAG_CHECK_OPTIMIZED_OFFSET(p_defrag_test->p_ncsi_start);

    /* How it works:
     * 
     * 1. We prepend a 32-bit `extra_byte` to the NC-SI packet, then set the start pointer 
     *    to an unaligned offset of 1 byte ahead, at `0x0001`.
     * 
     * 2. The first read of 63 bytes starts at an unaligned address (`0x0001`), incurring 
     *    a small performance penalty.
     * 
     * 3. After reading the initial 63 bytes, the next packet begins at an aligned 
     *    offset of 64 bytes, allowing the processor to use optimized 16-byte memory copy 
     *    instructions for the rest of the data.
     * 
     * 4. This approach ensures that, after the initial small penalty, all subsequent memory 
     *    operations are aligned, resulting in significantly faster processing and fewer cycles.
     */

    /* Use the 'frag' test ability to create MCTP fragments */
    if ( test_frag_prologue((uintptr_t) p_defrag_test->ncsi_packet_size) != 0 )
        return 1;

    test_exec_frag(0); /* Trigger MCTP fragments generation */

    /* Count received frames */
    LL_COUNT(p_defrag_test->p_usb_packets, packet, p_defrag_test->usb_packets_count);

    if ( p_defrag_test->usb_packets_count == 0 )
        return 1; /* No data */

    p_defrag_test->ncsi_packet_size -= 3;
    p_defrag_test->rx_raw_size = 0;

#if ( TEST_CONTINUOUS_MODE == 0 )
    printf("Input: %d USB buffers, total %d bytes.\n", p_defrag_test->usb_packets_count, p_defrag_test->usb_raw_size);
#endif
    return 0; /* Ready! */
}

/**
 * @brief Provides a description of the defragmentation test.
 *
 * This function returns a description of the defragmentation test based on the
 * specified description type. It helps in understanding the purpose and
 * functionality of the test being performed.
 *
 * @param description_type The type of description requested. If 0, a short
 *                         one-line description is returned. If 1, a detailed
 *                         description is provided.
 *
 * @return A string containing the description of the defragmentation test.
 */

char *test_defrag_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "'defrag' local implementation.";
    }
    else
    {
        return "Defragmentation test: This test simulates the reception of \n"
               "MCTP packets fragmented across multiple USB chunks. It \n"
               "validates the sequence numbers of the packets, reassembles \n"
               "them into a contiguous buffer, and checks for data integrity. \n"
               "The first packet is treated specially, where a specific byte \n"
               "is skipped after the MCTP header, and the payload is assembled \n"
               "from subsequent packets, ensuring the final packet is complete \n"
               "and accurate.";
    }
}

/**
 * @brief Initializes the defragmentation test module.
 *
 * This function allocates the necessary resources for the defragmentation
 * test module and initializes the fragment test with the specified USB TX
 * handler. It ensures that the module is prepared for further processing.
 *
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 *
 * @return 0 on success, else 1.
 */

int test_defrag_init(uintptr_t arg)
{
    int ret;

    if ( p_defrag_test != NULL )
        return 0; /* Module already initialized */

    /* Request RAM for this module, assert on failure */
    p_defrag_test = hal_alloc(sizeof(test_defrag_session));
    if ( p_defrag_test == NULL )
        return 1;

    p_defrag_test->req_packet_size = NCSI_PACKET_MAX_SIZE;

    /* Init the 'frag' test with our 'USB TX handler' */
    ret = test_frag_init((uintptr_t) test_defrag_on_usb_tx);

    return ret;
}
