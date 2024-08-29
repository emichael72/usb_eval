
/**
  ******************************************************************************
  *
  * @file ncsi_packet.h
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

#define NCSI_PACKET_MAX_SIZE  1536                            /* 1.5 KB maximum packet size */
#define NCSI_PAYLOAD_MAX_SIZE (NCSI_PACKET_MAX_SIZE - 14 - 1) /* Ethernet header  is 14 bytes + Intel prepended byte */
#define NCSI_PACKED           0

/* Ethernet header structure */
typedef struct
{
    uint8_t  dest_mac[6]; /* Destination MAC address (6 bytes) */
    uint8_t  src_mac[6];  /* Source MAC address (6 bytes) */
    uint16_t ethertype;   /* EtherType, should be 0x88F8 for NC-SI 
                                 (2 bytes) */
} ethernet_header_t;

/* NC-SI Command/Response packet structure */
typedef struct
{
    uint8_t  mc_id;                          /* Management Controller Identifier (4 bits) */
    uint8_t  header_revision;                /* Header Revision (4 bits) */
    uint16_t reserved1;                      /* Reserved (1 byte) */
    uint16_t payload_length;                 /* Payload Length (2 bytes) */
    uint8_t  channel_id;                     /* Channel Identifier (1 byte) */
    uint8_t  command_response;               /* Command/Response (1 byte) */
    uint16_t reserved2;                      /* Reserved (2 bytes) */
    uint8_t  payload[NCSI_PAYLOAD_MAX_SIZE]; /* Payload (up to 1.5 KB - 
                                               Ethernet header) */
} ncsi_packet_t;

/* Full NC-SI Ethernet packet structure */
#if ( NCSI_PACKED > 0 )
typedef struct __attribute__((packed)) ncsi_eth_packet
#else
typedef struct ncsi_eth_packet
#endif
{
    uint32_t          extra_byte; /* ToDo: Architecture - alignment issue? */
    ethernet_header_t eth_header; /* Ethernet header */
    ncsi_packet_t     ncsi_data;  /* NC-SI data including headers and payload */
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

ncsi_eth_packet *ncsi_request_packet(uint16_t *size);

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
