/*
 * packet.cpp
 * Implementation of ICMP packets' checksum and packet creation.
 */

#include "ping.h"

// init
ICMPPacket::ICMPPacket(int sequence, int process_id)
{
    // Initialize ICMP header
    header.type = ICMP_ECHO; // Type 8 (Echo Request)
    header.code = 0;
    header.checksum = 0;                            // Set to 0 first for checksum calculation
    header.un.echo.id = htons(process_id & 0xFFFF); // mark the packet with process ID
    header.un.echo.sequence = htons(sequence);

    // Fill data with current timestamp
    struct timeval tv;
    gettimeofday(&tv, NULL);
    snprintf(data, sizeof(data), "Timestamp: %ld.%06ld", tv.tv_sec, tv.tv_usec);

    // Calculate checksum
    calculate_checksum();
}

// Calculate checksum for the ICMP packet
// ref:
// https://blog.csdn.net/qq_37174526/article/details/88407884
void ICMPPacket::calculate_checksum()
{
    header.checksum = 0; // Reset checksum before calculation

    // Create a temporary buffer for checksum calculation
    char buffer[sizeof(header) + sizeof(data)];
    memcpy(buffer, &header, sizeof(header));
    memcpy(buffer + sizeof(header), data, sizeof(data));

    unsigned long sum = 0;
    unsigned short *addr = (unsigned short *)buffer;
    size_t len = sizeof(buffer);

    // Calculate checksum
    while (len > 1)
    {
        sum += ntohs(*addr++); // Convert to host byte order for calculation
        len -= 2;
    }

    // Handle odd byte if any
    if (len == 1)
    {
        sum += *(unsigned char *)addr << 8; // Left shift for big-endian order
    }

    // Add carry bits to low 16 bits
    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    header.checksum = htons(~sum); // Final checksum in network byte order
}