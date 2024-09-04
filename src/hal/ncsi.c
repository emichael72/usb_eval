
/**
  ******************************************************************************
  *
  * @file ncsi.c
  *  @brief Dummy API to help gemerating a relastic as posiiably NC SI packet.
  *  
  * @note https://www.dmtf.org/sites/default/files/standards/documents/DSP0222_1.0.0.pdf.
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
#include <ncsi.h>

/* Global instance of the NC-SI Ethernet packet */
static ncsi_eth_packet *p_ncsi = NULL;

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

ncsi_eth_packet *ncsi_request_packet(size_t *packet_size)
{
    int payload_size = 0;
    int total_size   = 0;

    /* Test only, allocation once */
    if ( p_ncsi != NULL || packet_size == NULL || *packet_size <= NCSI_HEADERS_SIZE || *packet_size > NCSI_PACKET_MAX_SIZE )
        return NULL;

    payload_size = NCSI_GET_PAYLOAD_SIZE(*packet_size);
    total_size   = *packet_size;
    *packet_size = 0;

    p_ncsi = (ncsi_eth_packet *) hal_alloc(total_size);
    if ( p_ncsi == NULL )
        return NULL;

    /* Populate the Ethernet header with realistic data */
    uint8_t example_dest_mac[6] = {0x00, 0x25, 0x90, 0xAB, 0xCD, 0xEF};
    uint8_t example_src_mac[6]  = {0x00, 0x14, 0x22, 0x01, 0x23, 0x45};

    memcpy(p_ncsi->eth_header.dest_mac, example_dest_mac, 6);
    memcpy(p_ncsi->eth_header.src_mac, example_src_mac, 6);
    p_ncsi->eth_header.ethertype = ncsi_htons(0x88F8); /* NC-SI EtherType */

    /* Populate NC-SI command/response header with realistic data */
    p_ncsi->ncsi_data.mc_id          = 0xA5;                     /* Example MC ID */
    p_ncsi->ncsi_data.command        = 0x01;                     /*  Select Package Command */
    p_ncsi->ncsi_data.channel_id     = 0x02;                     /* Channel ID */
    p_ncsi->ncsi_data.reserved       = 0x03;                     /* Reserved */
    p_ncsi->ncsi_data.payload_length = ncsi_htons(payload_size); /* Payload length */
    p_ncsi->ncsi_data.reserved2      = 0x5A;                     /* Reserved*/

#if ( HAL_PAINT_PACKET > 0 )
    /* Populate payload with example data, this data could be later validated */
    int ret = hal_paint_buffer(p_ncsi->ncsi_data.payload, payload_size);
    if ( ret != 0 )
        return NULL;
#endif

    *packet_size = total_size;
    return p_ncsi;
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
    if ( pkt != NULL )
    {
        /* Placeholder: no action needed since we're using a statically installed packet 
         * rather than dynamically generating it.
         */

        hal_zero_buf(pkt, sizeof(ncsi_eth_packet));
    }
}
