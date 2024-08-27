
/**
  ******************************************************************************
  *
  * @file ncsi_packet.c
  *  @brief NC-SI Ethernet packet structure definitions.
  *
  * Dummy API to help gemerating a relastic as posiiably NC SI packet.
  *  
  * @note This file includes guards to prevent recursive inclusion.
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
#include <string.h>
#include <ncsi_packet.h>

/* Global instance of the NC-SI Ethernet packet */
static ncsi_eth_packet ncsi_global_packet = {0};

/**
 * @brief Converts a 16-bit value from host byte order to network byte order.
 * @param hostshort The 16-bit value in host byte order.
 * @return The value in network byte order.
 */

static uint16_t ncsi_htons(uint16_t hostshort)
{
    return (hostshort << 8) | (hostshort >> 8);
}

/**
 * @brief Requests and returns a pointer to a global NC-SI Ethernet packet.
 *
 * This function populates the global instance of the NC-SI Ethernet packet
 * with realistic data, such as MAC addresses and example payload, and returns
 * a pointer to the populated packet.
 *
 * @return Pointer to the populated ncsi_eth_packet_t structure.
 */

ncsi_eth_packet *ncsi_request_packet(uint16_t *size)
{
    /* Populate the Ethernet header with realistic data */
    uint8_t example_dest_mac[6] = {0x00, 0x25, 0x90, 0xAB, 0xCD, 0xEF};
    uint8_t example_src_mac[6]  = {0x00, 0x14, 0x22, 0x01, 0x23, 0x45};

    memcpy(ncsi_global_packet.eth_header.dest_mac, example_dest_mac, 6);
    memcpy(ncsi_global_packet.eth_header.src_mac, example_src_mac, 6);
    ncsi_global_packet.eth_header.ethertype = ncsi_htons(0x88F8); /* NC-SI EtherType */

    /* Populate NC-SI command/response header with realistic data */
    ncsi_global_packet.ncsi_data.mc_id            = 0x01;           /* Example MC ID */
    ncsi_global_packet.ncsi_data.header_revision  = 0x01;           /* Example Header Revision */
    ncsi_global_packet.ncsi_data.payload_length   = ncsi_htons(64); /* Example payload length */
    ncsi_global_packet.ncsi_data.channel_id       = 0x00;           /* Example Channel ID */
    ncsi_global_packet.ncsi_data.command_response = 0x01;           /* Example Command */

    /* Populate payload with example data */
    memset(ncsi_global_packet.ncsi_data.payload, 0xA5, 64); /* Example payload data */

    if ( size != NULL )
        *size = sizeof(ncsi_global_packet);

    return &ncsi_global_packet;
}

/**
 * @brief Releases an NC-SI Ethernet packet.
 *
 * This function takes a pointer to an NC-SI Ethernet packet and performs no
 * operations. It is included to match the request/release pattern.
 *
 * @param pkt Pointer to the ncsi_eth_packet_t structure to be released.
 */

void ncsi_release_packet(ncsi_eth_packet *pkt)
{
    /* Do nothing */
}
