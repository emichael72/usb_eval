
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
#include <ncsi_packet.h>
#include <test_frag,h>
#include <test_defrag.h>
#include <string.h>

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

/* Full NC-SI Ethernet packet structure */

typedef struct __attribute__((packed)) ncsi_raw_packet_t
{

    ethernet_header_t eth_header; /* Ethernet header */
    ncsi_packet_t     ncsi_data;  /* NC-SI data including headers and payload */

} ncsi_raw_packet;

/* 
 * A single USB packet
 */

typedef struct _usb_packet_t
{
    struct _usb_packet_t *next; /* Pointer to the next packet */
    void                 *data; /* Frame data */
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
    uint8_t        *p_ncsi_start;      /**< Pointer to the start of the NC-SI message buffer. */
    usb_packet     *p_usb_packets;     /**< Head pointer to the linked list of USB packet fragments. */
    uint16_t        ncsi_packet_size;  /**< Expected size of the complete NC-SI packet in bytes. */
    uint16_t        rx_raw_size;       /**< Total size of the received raw data after defragmentation. */
    uint16_t        usb_raw_size;      /**< Total size of the raw data received from USB packets. */
    uint8_t         usb_packets_count; /**< Number of USB packet fragments received. */
    ncsi_raw_packet ncsi_packet;       /**< Buffer to store the assembled NC-SI Ethernet packet. */

} test_defrag_session;

/* Pointer to the module's session instance */
test_defrag_session *p_defrag = NULL;

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
    /* Collect all fragments into long USB frames, each containing multiple MCTP frames.
       These frames will serve as the inputs for the defragmentation test.
    */
    usb_packet *packet;
    size_t      total_size = 0;
    size_t      offset     = 0;

    if ( pairs == NULL || pairs_count == 0 )
        assert(0);

    /* Create 'USB' list item */
    packet = hal_alloc(sizeof(usb_packet));
    assert(packet != NULL);

    /* Get the total size of all the data included with the pairs */
    for ( int i = 0; i < pairs_count; i++ ) total_size += pairs[i].size;

    p_defrag->usb_raw_size += total_size; /*  Get the total sizer ofd all chunks */

    packet->data = hal_alloc(total_size);
    assert(packet->data != NULL);

    packet->size = total_size;

    /* Append each pairs->ptr to packet->data */
    for ( int i = 0; i < pairs_count; i++ )
    {
        memcpy(packet->data + offset, (void *) pairs[i].ptr, pairs[i].size);
        offset += pairs[i].size;
    }

    /* Attach the aggregated packet to the packets list */
    LL_APPEND(p_defrag->p_usb_packets, packet);
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

int test_defrag_prolog(uintptr_t arg)
{
    usb_packet *packet;

    if ( p_defrag == NULL )
        return 1; /* Module not initialized */

    p_defrag->usb_packets_count = 0;
    p_defrag->usb_raw_size      = 0;

    /* Make use of the 'frag' test ability to create MCTP fragments */
    /* Call the 'frag' test prolog */
    if ( test_frag_prolog(0) != 0 )
        return 1;

    test_exec_frag(0); /* Trigger MCTP fragments generation */

    /* Count received frames */
    LL_COUNT(p_defrag->p_usb_packets, packet, p_defrag->usb_packets_count);

    if ( p_defrag->usb_packets_count == 0 )
        return 1; /* No data */

    p_defrag->ncsi_packet_size = sizeof(ncsi_raw_packet);
    p_defrag->rx_raw_size      = 0;

    return 0; /* Ready! */
}

/**
 * @brief
 */

/**
 * @brief Defragments MCTP packets from USB chunks into a contiguous buffer.
 *
 * This function processes a series of USB packets, each containing MCTP packets.
 * It validates the MCTP sequence numbers and assembles the MCTP payloads into
 * a contiguous buffer. The first packet is treated specially, where a byte '3'
 * is skipped after the MCTP header.
 *
 * @param arg Unused parameter, provided for compatibility with the calling
 *            convention.
 */

void test_exec_defrag(uintptr_t arg)
{
    usb_packet *packet;
    uint8_t    *ncsi_packet_ptr   = (uint8_t *) &p_defrag->ncsi_packet;
    size_t      offset            = 0; /* Offset to track position in the buffer */
    int         first_chunk       = 1; /* Flag to indicate the first chunk */
    uint8_t     expected_sequence = 0; /* Expected MCTP sequence number */

    LL_FOREACH(p_defrag->p_usb_packets, packet)
    {
        size_t packet_offset  = 0;
        size_t remaining_size = packet->size;

        while ( remaining_size > 0 )
        {
            mctp_packet *mctp = (mctp_packet *) (packet->data + packet_offset);
            size_t       payload_size;
            size_t       header_size = 4; /* MCTP header size */

            /* Validate the MCTP sequence number */
            if ( mctp->packet_sequence != expected_sequence )
            {
#ifdef DEBUG
                printf("Error: packet dropped, sequence number mismatch.\n");
                return;
#endif
            }

            if ( first_chunk )
            {
                /* Validate and skip the first byte after the MCTP header */
                if ( mctp->payload[0] != 3 )
                {
#ifdef DEBUG
                    printf("Error: first byte after the MCTP header must be 3.\n");
                    return;
#endif
                }

                /* Calculate the payload size */
                payload_size = (mctp->end_of_message) ? (remaining_size - header_size - 1) : 62;

                /* Copy the payload, skipping the first byte */
                memcpy(ncsi_packet_ptr + offset, mctp->payload + 1, payload_size);

                /* Update offsets and flags */
                offset += payload_size;
                packet_offset += header_size + 63; /* 4 bytes header + 63 bytes */
                remaining_size -= header_size + 63;

                first_chunk = 0;
            }
            else
            {
                /* Calculate the payload size for normal packets */
                payload_size = (mctp->end_of_message) ? (remaining_size - header_size) : 64;

                /* Copy the payload directly */
                memcpy(ncsi_packet_ptr + offset, mctp->payload, payload_size);

                /* Update offsets */
                offset += payload_size;
                packet_offset += header_size + payload_size;
                remaining_size -= header_size + payload_size;
            }

            /* Increment the expected sequence number and handle wrap-around */
            expected_sequence = (expected_sequence + 1) & 0x03;
        }
    }

    /* Final size validation */
    if ( offset != p_defrag->ncsi_packet_size )
    {
#ifdef DEBUG
        printf("Error: The assembled packet size does not match the expected size.\n");
        return;
#endif
    }

#ifdef DEBUG
    printf("Success: Assembled packet passed all tests.\n");
#endif
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
        return "Defragmentation test: Assembles MCTP fragments.";
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

    if ( p_defrag != NULL )
        return 1; /* Module already initialized */

    /* Request RAM for this module, assert on failure */
    p_defrag = hal_alloc(sizeof(test_defrag_session));
    if ( p_defrag == NULL )
        return 1;

    /* Init the 'frag' test with our 'USB TX handler' */
    ret = test_frag_init((uintptr_t) test_defrag_on_usb_tx);

    return ret;
}
