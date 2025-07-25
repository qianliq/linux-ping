/*
 * ping.h
 * This file is major part of the Ping project.
 * Object-oriented implementation of the Ping class, see ping/client.cpp etc.
 *
 * WARNING: the code base on the system automatically add the IP header
 * and using the raw socket, so the code here is only for the ICMP header.
 */

#ifndef PING_H
#define PING_H

#include <iostream>
#include <string>
#include <cstring>
#include <limits>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
/*
 * ICMP packet structure
 * This structure represents an ICMP packet with a header and data.
 * It includes methods for calculating the checksum and getting the total size.
 *
 * parameters:
 * - sequence: the sequence number of the ICMP packet
 * - process_id: the process ID of the ping client
 */
struct ICMPPacket
{
    struct icmphdr header;
    char data[56];

    ICMPPacket(int sequence, int process_id);
    void calculate_checksum();
    size_t total_size() const { return sizeof(header) + sizeof(data); }
};

/*
 * ping statistics class
 * This class is used to keep track of the ping statistics.
 * It records the number of packets sent, received and response times.
 * When the pinging is done(end or Ctrl+C), it prints the statistics.
 */
class PingStatistics
{
private:
    int packets_sent_;
    int packets_received_;
    double total_time_;
    double min_time_;
    double max_time_;

public:
    PingStatistics();

    // Update statistics
    void add_response_time(double time_ms);
    void increment_sent() { packets_sent_++; }
    void increment_received() { packets_received_++; }

    // Get statistics
    int packets_sent() const { return packets_sent_; }
    int packets_received() const { return packets_received_; }
    int packets_lost() const { return packets_sent_ - packets_received_; }
    double loss_rate() const
    {
        return (packets_sent_ > 0) ? (static_cast<double>(packets_lost()) / packets_sent_) * 100.0 : 0.0;
    };
    double average_time() const
    {
        return (packets_received_ > 0) ? total_time_ / packets_received_ : 0.0;
    };
    double min_time() const { return min_time_; }
    double max_time() const { return max_time_; }

    void print_statistics(const std::string &target_ip) const;
};

/*
 * ping client class
 * This class is the core of the ping functionality.
 * It handles:
 * - Parsing command line arguments
 * - Creating and managing the socket
 * - The creation of ICMP packets, sending them, receiving replies and calculating statistics.
 */
class PingClient
{
private:
    std::string target_ip_;        // string for target IP address
    struct sockaddr_in dest_addr_; // sockaddr_in structure for destination address

    int process_id_; // Process ID for the ICMP header
    int socket_fd_;  // Socket file descriptor for raw socket
    int count_;      // number of packets to send
    int interval_;   // interval between packets in milliseconds
    double timeout_ms_; // timeout for receiving reply in milliseconds
    bool running_;   // flag to indicate if pinging is running

    PingStatistics stats_;

    bool create_socket();
    bool setup_target_address();
    void close_socket();
    double calculate_time_diff(const struct timeval &start, const struct timeval &end);
    bool send_ping_packet(int sequence);
    bool receive_ping_reply(int sequence);
    bool drop_ping_reply();

public:
    PingClient(const std::string &target_ip, int count, int interval, double timeout_ms);
    ~PingClient();

    bool initialize();
    void run();
    void stop() { running_ = false; }

    static bool parse_arguments(int argc, char *argv[], std::string &target_ip, int &count, int &interval, double &timeout_ms);
    static void print_usage(const char *prog_name);
};

#endif // PING_H