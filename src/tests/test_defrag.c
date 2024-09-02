
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
#include <ncsi_packet.h>
//#include <test_defrag.h>
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

typedef struct test_defrag_session_t
{
    ncsi_eth_packet *p_ncsi_packet; /**< Pointer to the current NC-SI Ethernet packet. */
    uint8_t         *p_ncsi_start;  /**< Pointer to the start of the NC-SI message. */

} test_defrag_session;

/* Pointer to the module's session instance */
test_defrag_session *p_defrag = NULL;

/**
 * @brief 
 */

int test_defrag_prolog(uintptr_t arg)
{
    return 0;
}

/**
 * @brief
 */

void test_exec_defrag(uintptr_t arg)
{
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
        return 0;

    return 1;
}
