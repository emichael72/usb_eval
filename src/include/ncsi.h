
/**
  ******************************************************************************
  *
  * @file ncsi.h
  *  @brief NC-SI Ethernet packet structure definitions.
  *
  * This file defines the structures required to handle NC-SI Ethernet packets.
  * The structures include the Ethernet header, NC-SI command/response header, 
  * and the full NC-SI Ethernet packet. The structures are designed to be 
  * compliant with the NC-SI specification and optimized for processing with 
  * minimal memory operations. 
  *  
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NCSI_PACKET_H
#define NCSI_PACKET_H

#include <stdint.h>
#define NCSI_INTEL_PRE_BYTE                 4                          /* Prepended MCTP byte(s) */
#define NCSI_PACKET_MAX_SIZE                1500 + NCSI_INTEL_PRE_BYTE /* MAX MTU */
#define NCSI_HEADERS_SIZE                   (sizeof(ncsi_eth_packet))
#define NCSI_PAYLOAD_MAX_SIZE               (NCSI_PACKET_MAX_SIZE - NCSI_HEADERS_SIZE)
#define NCSI_GET_PACKET_SIZE(payload)       (payload + NCSI_HEADERS_SIZE)
#define NCSI_GET_PAYLOAD_SIZE(packet_size)  (packet_size - NCSI_HEADERS_SIZE)
#define NCSI_GET_PAYLOAD_CLEAN(packet_size) (packet_size - NCSI_HEADERS_SIZE + (NCSI_INTEL_PRE_BYTE - 1))

/* Ethernet 14 bytes header structure */
typedef struct __attribute__((packed)) ethernet_header_t
{
    uint8_t  dest_mac[6]; /* Destination MAC address (6 bytes) */
    uint8_t  src_mac[6];  /* Source MAC address (6 bytes) */
    uint16_t ethertype;   /* EtherType, should be 0x88F8 for NC-`SI (2 bytes) */
} ethernet_header;

/* NC-SI 8 bytes Command/Response packet structure */
typedef struct __attribute__((packed)) ncsi_packet_t
{
    uint8_t  mc_id;          /* Management Controller (MC) ID */
    uint8_t  command;        /* NC-SI Command or Response */
    uint8_t  channel_id;     /* Channel ID (bits 7:4 - Package ID, bits 3:0 - Channel ID) v*/
    uint8_t  reserved;       /* Reserved (must be 0) */
    uint16_t payload_length; /* Length of the payload (in bytes) - 2 bytes */
    uint16_t reserved2;      /* Reserved (must be 0) - 2 bytes */
    uint8_t  payload[];      /* Variable-length payload (optional, depends on command) */
} ncsi_packet;

/* Full NC-SI Ethernet packet structure { 4 + 14 + 8 = 26 ...} */

typedef struct __attribute__((packed)) ncsi_eth_packet_t
{
#if NCSI_INTEL_PRE_BYTE > 0
    uint8_t extra_byte[NCSI_INTEL_PRE_BYTE]; /* Prepend intel byte. */
#endif
    ethernet_header eth_header; /* Ethernet header */
    ncsi_packet     ncsi_data;  /* NC-SI data including headers and payload */

} ncsi_eth_packet;

/**
 * @brief Requests and returns a pointer to a global NC-SI Ethernet packet.
 *
 * This function populates the global instance of the NC-SI Ethernet packet
 * with realistic data, such as MAC addresses and example payload, and returns
 * a pointer to the populated packet.
 *
 * @return Pointer to the populated ncsi_eth_packet_t structure.
 */

ncsi_eth_packet *ncsi_request_packet(size_t *packet_size);

/**
 * @brief Releases an NC-SI Ethernet packet.
 *
 * This function takes a pointer to an NC-SI Ethernet packet and performs no
 * operations. It is included to match the request/release pattern.
 *
 * @param pkt Pointer to the ncsi_eth_packet_t structure to be released.
 */
void ncsi_release_packet(ncsi_eth_packet *pkt);

#endif /* NCSI_PACKET_H */
