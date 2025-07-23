/*
 * ping.h
 * This file is major part of the Ping project.
 * Object-oriented implementation of the Ping class, see ping/***.cpp
 *
 * WARNING: the code base on the system automatically add the IP header
 * and using the raw socket, so the code here is only for the ICMP header.
 */

#ifndef PING_H
#define PING_H

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>
#include <cstdlib>

// handling ping instance for signal handling
class PingClient;
extern PingClient *g_ping_instance;

// Ctrl+C then stop pinging
void signal_handler(int signum)
{
    if (signum == SIGINT && g_ping_instance)
    {
        g_ping_instance->stop();
    }
}

/*
* ICMP packet structure
* This structure represents an ICMP packet with a header and data.
* It includes methods for calculating the checksum and getting the total size.
* 
* parameters:
* - sequence: the sequence number of the ICMP packet
* - process_id: the process ID of the ping client
*/
struct ICMPPacket {
    struct icmphdr header;
    char data[56];
    
    ICMPPacket(int sequence, int process_id);
    void calculate_checksum();
    size_t total_size() const { return sizeof(header) + sizeof(data); }
};

/*
* ping statistics class
* This class is used to keep track of the ping statistics.
* It records the number of packets sent, received, lost etc.
* When the pinging is done(end or Ctrl+C), it prints the statistics.
*/
class PingStatistics {
private:
    int packets_sent_;
    int packets_received_;
    double total_time_;
    double min_time_;
    double max_time_;
    
public:
    PingStatistics();
    
    void add_response_time(double time_ms);
    void increment_sent() { packets_sent_++; }
    void increment_received() { packets_received_++; }
    
    // Getters
    int packets_sent() const { return packets_sent_; }
    int packets_received() const { return packets_received_; }
    int packets_lost() const { return packets_sent_ - packets_received_; }
    double loss_rate() const;
    double average_time() const;
    double min_time() const { return min_time_; }
    double max_time() const { return max_time_; }
    
    void print_statistics(const std::string& target_ip) const;
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
    std::string target_ip_;
    struct sockaddr_in dest_addr_;

    int socket_fd_;
    int count_;
    int interval_;
    bool running_;

    PingStatistics stats_;

    // Private methods
    bool create_socket();
    bool setup_target_address();
    void close_socket();
    unsigned short calculate_checksum(void *data, size_t len);
    double calculate_time_diff(const struct timeval &start, const struct timeval &end);
    bool send_ping_packet(int sequence);
    bool receive_ping_reply(int sequence, double &delay_ms, const struct timeval &send_time);

public:
    PingClient(const std::string &target_ip, int count, int interval);
    ~PingClient();

    bool initialize();
    void run();
    void stop() { running_ = false; }

    static bool parse_arguments(int argc, char *argv[], std::string &target_ip, int &count, int &interval);
    static void print_usage(const char *prog_name);

    // Getters
    const std::string &target_ip() const { return target_ip_; }
}

#endif // PING_H