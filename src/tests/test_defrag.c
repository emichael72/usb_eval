
/**
  ******************************************************************************
  * @file    test_defrag_mctplib.c
  * @author  IMCv2 Team
  * @brief   Aggregates 25 fragments of MCTP packets into a single NC-SI.
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

#define member_size(type, member) (sizeof(((type *) 0)->member))

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
 * @brief Holds all global variables for the module.
 */

typedef struct test_defrag_session_t
{
    uint8_t        *p_ncsi_start;     /**< Pointer to the start of the NC-SI message. */
    usb_packet     *p_usb_packets;    /**< Head pointer of all our USB fragments. */
    uint16_t        ncsi_packet_size; /**< Size of the NC-SI packet in bytes. */
    ncsi_raw_packet ncsi_packet;      /**< Assembeled NC-SI Ethernet packet. */
    uint16_t        rx_raw_size;
    uint16_t        usb_raw_size;
    uint8_t         usb_packets_count;

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

static void test_defragfrag_on_usb_tx(ptr_size_pair *pairs, size_t pairs_count)
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
 * @brief 
 */

int test_defrag_prolog(uintptr_t arg)
{
    usb_packet *packet;

    if ( p_defrag == NULL )
        return 1; /* Module not initalized */

    p_defrag->usb_packets_count = 0;
    p_defrag->usb_raw_size      = 0;

    /* We're making use of the 'frag' test ability to create MCTP fragments */
    /* Call the 'frag' test prolog */
    if ( test_frag_prolog(0) != 0 )
        return 1;

    test_exec_frag(0); /* Trigger MCTP fragments generation */

    /* Count reviced frames */
    LL_COUNT(p_defrag->p_usb_packets, packet, p_defrag->usb_packets_count);

    if ( p_defrag->usb_packets_count == 0 )
        return 1; /* No data */

    p_defrag->ncsi_packet_size = sizeof(ncsi_raw_packet);
    p_defrag->rx_raw_size      = 0;

    return 0; /* Ready !*/
}

/**
 * @brief
 */

void test_exec_defrag(uintptr_t arg)
{
    usb_packet *packet;
    uint8_t    *ncsi_packet_ptr = (uint8_t *) &p_defrag->ncsi_packet; /* Pointer to the global buffer for the final defragmented packet */
    size_t      offset          = 0;                                  /* Offset to track position in the global buffer */
    int         first_chunk     = 1;                                  /* Flag to indicate the first chunk */

    LL_FOREACH(p_defrag->p_usb_packets, packet)
    {
        size_t packet_offset = 0;

        while ( packet_offset < packet->size )
        {
            /* Extract the MCTP header */
            mctp_packet *mctp = (mctp_packet *) (packet->data + packet_offset);

            size_t payload_size;

            if ( first_chunk )
            {
                /* First packet: Skip the first byte after the MCTP header, which is '3' */
                if ( mctp->payload[0] != 3 )
                {
                    assert(0); /* The first byte after the MCTP header must be 3 */
                }

                /* The payload size for the first packet is up to 62 bytes, excluding the '3' */
                payload_size = (mctp->end_of_message) ? (packet->size - packet_offset - 5) : 62;

                /* Copy the payload, skipping the first byte */
                memcpy(ncsi_packet_ptr + offset, mctp->payload + 1, payload_size);

                /* Update the offset */
                offset += payload_size;

                first_chunk = 0;

                /* Move to the next MCTP packet within the USB packet */
                packet_offset += 67; /* 4 bytes for the MCTP header + 63 bytes payload (including the special byte) */
            }
            else
            {
                /* Normal packets: payload size is 64 bytes unless it's the last packet */
                payload_size = (mctp->end_of_message) ? (packet->size - packet_offset - 4) : 64;

                /* Copy the payload directly */
                memcpy(ncsi_packet_ptr + offset, mctp->payload, payload_size);

                /* Update the offset */
                offset += payload_size;

                /* Move to the next MCTP packet within the USB packet */
                packet_offset += 4 + payload_size; /* 4 bytes for the MCTP header plus the payload */
            }
        }
    }

    /* Validate final assembled packet size */
    if ( offset != p_defrag->ncsi_packet_size )
    {
        assert(0); /* The assembled packet size does not match the expected size */
    }
}

/**
 * @brief
 */

char *test_defrag_desc(size_t description_type)
{
    if ( description_type == 0 )
    {
        return "Defragmentation test.";
    }
    else
    {
        return "\n";
    }
}

/**
 * @brief
  @return 0 on sucsess, else 1.
 */

int test_defrag_init(uintptr_t arg)
{

    if ( p_defrag != NULL )
        return 1;

    /* Request RAM for this module, assert on failure */
    p_defrag = hal_alloc(sizeof(test_defrag_session));
    if ( p_defrag == NULL )
        return 1;

    /* Init the 'frag' test with our 'USB TX hanlder' */
    return test_frag_init((uintptr_t) test_defragfrag_on_usb_tx);
}
