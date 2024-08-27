
/**
  ******************************************************************************
  *
  * @file frag.c
  * @brief Exploding an NCSI ethetnet packet to mutiple frames.
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
#include <stdint.h>

#define USB_FRAME_SIZE   64 /* USB base packet size */
#define MCTP_HEADER_SIZE 4

/* Maximum size in byts for a single fragment */
#define MCTP_MAX_FRAGMNET_SIZE (USB_FRAME_SIZE + MCT_HEADER_SIZE)

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
typedef struct mctp_packet_t
{
    uint8_t version;              /* MCTP version */
    uint8_t destination_eid;      /* Destination Endpoint ID */
    uint8_t source_eid;           /* Source Endpoint ID */
    uint8_t message_tag      : 3; /* Message Tag */
    uint8_t tag_owner        : 1; /* Tag Owner */
    uint8_t packet_sequence  : 2; /* Packet Sequence Number */
    uint8_t end_of_message   : 1; /* End of Message Indicator */
    uint8_t start_of_message : 1; /* Start of Message Indicator */
    uint8_t integrity_check  : 4; /* Integrity Check */
    uint8_t reserved         : 4; /* Reserved */

} mctp_ptr_packet;

/* 
 * MCTP and additional pointers packet structure
 * The MCTP header is fixed at 4 bytes (32 bits), so our additional pointers 
 * placed after the header at the end should not pose a problem.
 */

typedef struct mctp_frag_t
{

    mctp_ptr_packet     mctp_header;  /* MCTP 8 bytes header */
    uint8_t *           payload;      /* Pointer to the actual NC-SI data */
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
    ncsi_eth_packet *p_ncsi_packet;    /**< Pointer to the currently handled  NC-SI Ethernet packet. */
    mctp_frag *      p_mctp_head;      /**< Head pointer to the MCTP fragments list. */
    uint8_t          ncsi_frgas_count; /**< Count of MCTP fragments used to  encapsulate the NC-SI packet. */
    uint16_t         ncsi_packet_size; /**< Size of the inbound NC-SI packet in bytes. */
    uint8_t          version;          /**< MCTP header version. */
    uint8_t          destination_eid;  /**< MCTP destination EID. */
    uint8_t          source_eid;       /**< MCTP source EID. */

} frag_test;

frag_test *p_frag_test = NULL;

/**
 * @brief Adjusts the pointers in the fragment list to correspond to a new 
 *        NC-SI packet.
 *
 * This function is called whenever a new NC-SI packet is received. It updates
 * the fragment list to point to the appropriate sections of the NC-SI packet
 * and adjusts the payload sizes accordingly. The last fragment in the list is 
 * marked with the end-of-message (EOM) flag.
 */

static void frag_test_adjust_pointers(void)
{
    mctp_frag *frag            = p_frag_test->p_mctp_head;
    uint8_t *  p_payload       = (uint8_t *) p_frag_test->p_ncsi_packet;
    size_t     remaining_bytes = p_frag_test->ncsi_packet_size;

    /* Adjust the fragments list to the inbound NC-SI packet */
    for ( uint16_t i = 0; i < p_frag_test->ncsi_frgas_count && frag; i++ )
    {
        /* Set the payload pointer and size */
        frag->payload      = p_payload;
        frag->payload_size = (remaining_bytes > USB_FRAME_SIZE) ? USB_FRAME_SIZE : remaining_bytes;

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
 * @brief Obtain a fake Ethernet NC-SI frame along with its size.
 *
 * In a real-world scenario, this frame would be placed in a designated RAM 
 * region, and this logic would be notified via an interrupt.
 *
 * @note TBD: Confirm if this is the correct approach.
 *
 * @return 0 on success, 1 on failure.
 */

int frag_test_on_ncsi_rx(void)
{
    
    if(p_frag_test == NULL)
        return 1; /* Module not initalized */
    
    p_frag_test->p_ncsi_packet = ncsi_request_packet(&p_frag_test->ncsi_packet_size);
    if(p_frag_test->p_ncsi_packet == NULL || p_frag_test->ncsi_packet_size == 0)
        return 1;

    /* Prepended '3'to NCSI packet */
    p_frag_test->p_ncsi_packet->extra_byte = 3;

    /* Calculate the total number of fragments needed */
    p_frag_test->ncsi_frgas_count = (p_frag_test->ncsi_packet_size + USB_FRAME_SIZE - 1) / USB_FRAME_SIZE;

    if ( p_frag_test->ncsi_frgas_count > MCTP_MAX_FRAGMENTS )
    {
        /* Drop the packet , it's too big */
        p_frag_test->ncsi_frgas_count = 0;
        return 1;
    }

    return 0; /* NC-SI packet reday for frgmantation */
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

void frag_usb_tx(ptr_size_pair *pairs, size_t pairs_count)
{
    /* Implementation of USB transmission would go here. */
}

/**
 * @brief Starts the fragments test by processing and transmitting NC-SI 
 *        packet fragments.
 *
 * This function is executed when a new NC-SI packet is available. It adjusts 
 * the fragment pointers using `frag_test_adjust_pointers()`, and then iterates 
 * through the fragments, preparing batches of MCTP headers and their 
 * corresponding payloads. These batches are then sent to the USB hardware 
 * in groups of up to 4 fragments (8 pairs of header and payload).
 *
 * @param arg Unused parameter, reserved for future use.
 */

void frag_test_start(uintptr_t arg)
{
    /* By now, we trust that there is a pending NC-SI packet */
    frag_test_adjust_pointers();

    /* Define an array to hold up to 4 pairs at a time */
    ptr_size_pair pairs[8]; // Each fragment has two pairs: header and payload

    /* Iterate over the fragments and send them in batches */
    mctp_frag *frag        = p_frag_test->p_mctp_head;
    size_t     pairs_count = 0;

    for ( int i = 0; i < p_frag_test->ncsi_frgas_count; i++ )
    {
        /* First pair: MCTP header and its size */
        pairs[pairs_count].ptr  = (uintptr_t) &frag->mctp_header;
        pairs[pairs_count].size = sizeof(frag->mctp_header);
        pairs_count++;

        /* Second pair: Payload pointer and its size */
        pairs[pairs_count].ptr  = (uintptr_t) frag->payload;
        pairs[pairs_count].size = frag->payload_size;
        pairs_count++;

        /* Move to the next fragment */
        frag = frag->next;

        /* Check if we've collected 4 fragments (8 pairs) or reached 
           the last fragment */
        if ( pairs_count == 8 || i == p_frag_test->ncsi_frgas_count - 1 )
        {
            /* Send the batch to the USB hardware */
            frag_usb_tx(pairs, pairs_count);
            pairs_count = 0; /* Reset the counter for the next batch */
        }
    }
}

/*
 * Initializes the module, allocates RAM for persistent variables, 
 * and sets the required prerequisites for the test.
 */

void frag_test_init(void)
{
    mctp_frag *frag      = NULL;
    int        msg_index = 0;

    if ( p_frag_test != NULL )
        return; /* Must initialize only once */

    xos_disable_interrupts();

    /* The following operations are performed once and do not count as 
     * 'test-related cycles'. 
     */

    p_frag_test = hal_alloc(sizeof(frag_test));
    assert(p_frag_test != NULL);

    /* Sets MCTP defaults */
    p_frag_test->version         = 1;
    p_frag_test->destination_eid = 0x10;
    p_frag_test->source_eid      = 0x20;
    p_frag_test->p_mctp_head     = NULL;

    /* Allocate all MCTP fragments and attch them to the session head pointer.
    For now, feel as many fealds as posiiable to reduice setup clocks at run-time. */
    while ( msg_index < MCTP_MAX_FRAGMENTS )
    {

        frag = hal_alloc(sizeof(mctp_frag));
        assert(frag != NULL);

        /* Initialize MCTP header */
        frag->mctp_header.version         = p_frag_test->version;
        frag->mctp_header.destination_eid = p_frag_test->destination_eid;
        frag->mctp_header.source_eid      = p_frag_test->source_eid;
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

        frag->mctp_header.end_of_message  = 0;   /* Not End of Message */
        frag->mctp_header.integrity_check = 0xF; /* Dummy integrity check */
        frag->mctp_header.reserved        = 0;   /* Reserved field */

        /* Those are not set initially */
        frag->payload      = NULL;
        frag->payload_size = 0;

        /* Attach to the galobal head pointert */
        LL_APPEND(p_frag_test->p_mctp_head, frag);
        
        msg_index++;
    }

    /* We'rte ready. */
}
