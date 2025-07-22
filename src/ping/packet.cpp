/*
 * packet.cpp
 * Implementation of ICMP packets' checksum and packet creation.
 */

#include "ping.h"

// init
ICMPPacket::ICMPPacket(int sequence, int process_id) {
    // Initialize ICMP header
    header.type = ICMP_ECHO; // Type 8 (Echo Request)
    header.code = 0;
    header.checksum = 0;  // Set to 0 first for checksum calculation
    header.un.echo.id = htons(process_id & 0xFFFF);// mark the packet with process ID
    header.un.echo.sequence = htons(sequence);

    // Fill data with 'A'
    memset(data, 'A', sizeof(data));
    
    // Calculate checksum
    calculate_checksum();
}