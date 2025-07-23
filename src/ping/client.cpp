/*
 * client.cpp
 * Implementation of ping client functionalities.
 */

#include "ping.h"

// init
// only init the data
PingClient::PingClient(const std::string &target_ip, int count, int interval)
    : target_ip_(target_ip), process_id_(getpid()), socket_fd_(-1), count_(count),
      interval_(interval), running_(false)
{
    memset(&dest_addr_, 0, sizeof(dest_addr_));
}

PingClient::~PingClient()
{
    close_socket();
}

// Create a raw socket for ICMP
bool PingClient::create_socket()
{
    socket_fd_ = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket_fd_ < 0)
    {
        perror("Socket creation failed");
        return false;
    }
    return true;
}

// Setup the target address for the ping,
// set the address family and convert the IP address from string to binary
bool PingClient::setup_target_address()
{
    // make sure the address is clean
    memset(&dest_addr_, 0, sizeof(dest_addr_));

    // set the address family and convert the IP address from string to binary
    dest_addr_.sin_family = AF_INET;
    if (inet_aton(target_ip_.c_str(), &dest_addr_.sin_addr) == 0)
    {
        fprintf(stderr, "Invalid IP address: %s\n", target_ip_.c_str());
        return false;
    }
    return true;
}

void PingClient::close_socket()
{
    if (socket_fd_ >= 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

double calculate_time_diff(const struct timeval &start, const struct timeval &end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
}

// Send a ping packet with the given sequence number
// Using process_id_ to mark the packet
bool PingClient::send_ping_packet(int sequence)
{
    ICMPPacket packet(sequence, process_id_);

    // Send the packet
    ssize_t bytes_sent = sendto(socket_fd_, &packet, sizeof(packet), 0,
                                (struct sockaddr *)&dest_addr_, sizeof(dest_addr_));
    if (bytes_sent < 0)
    {
        perror("Failed to send ICMP packet");
        return false;
    }

    stats_.increment_sent(); // send success
    return true;
}

bool PingClient::receive_ping_reply(int sequence, double &delay_ms, const struct timeval &send_time)
{
    char recv_buf[1024];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);

    // Receive the reply
    // Blocking call, will wait for a reply
    ssize_t bytes_received = recvfrom(socket_fd_, recv_buf, sizeof(recv_buf), 0,
                                      (struct sockaddr *)&recv_addr, &addr_len);

    if (bytes_received <= 0)
    {
        printf("Request timeout for icmp_seq=%d\n", sequence);
        return false;
    }

    // Record time immediately after receiving reply
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    // Calculate delay
    delay_ms = calculate_time_diff(send_time, end_time);

    // Parse received packet
    struct ip *ip_reply = (struct ip *)recv_buf;
    struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buf + (ip_reply->ip_hl * 4));

    // Verify if it's ICMP Echo Reply
    if (icmp_reply->type == ICMP_ECHOREPLY && ntohs(icmp_reply->un.echo.sequence) == sequence)
    {
        stats_.increment_received();
        stats_.add_response_time(delay_ms);
        // Print the reply information
        // bytes_received - (ip_reply->ip_hl * 4) to get the ICMP header size
        printf("%zd bytes from %s: icmp_seq=%d time=%.3f ms\n",
               bytes_received - (ip_reply->ip_hl * 4),
               inet_ntoa(recv_addr.sin_addr),
               ntohs(icmp_reply->un.echo.sequence),
               delay_ms);

        return true;
    }

    // If not an Echo Reply, print the type
    printf("Received ICMP packet with type %d (not Echo Reply)\n", icmp_reply->type);
    return false;
}

// Initialize the PingClient
// Include:
// - Create socket
// - Setup target address
// - Setup flags
bool PingClient::initialize()
{
    if (!create_socket())
    {
        return false;
    }

    if (!setup_target_address())
    {
        close_socket();
        return false;
    }

    running_ = true; // Set running flag to true
    return true;
}

void PingClient::run()
{
    if (!running_)
    {
        fprintf(stderr, "PingClient not initialized\n");
        return;
    }

    // Print the initial ping message
    printf("PING %s: 56 data bytes\n", target_ip_.c_str());

    // Send ping packets
    // If not running(Ctrl+C), stop sending packets
    for (int i = 1; i <= count_ && running_; i++)
    {
        // Record send time
        struct timeval send_time;
        gettimeofday(&send_time, NULL);

        // send and receive ping packet
        if (send_ping_packet(i))
        {
            double delay_ms;
            receive_ping_reply(i, delay_ms, send_time);
        }

        // Wait for interval time
        if (i < count_ && running_)
        {
            sleep(interval_);
        }
    }

    // Print final statistics
    stats_.print_statistics(target_ip_);
    // No need to stop the PingClient, the stop method is just for the print_statisticsand it's already called
}

bool PingClient::parse_arguments(int argc, char *argv[], std::string &target_ip, int &count, int &interval)
{
    // Default values
    count = 4;    // Default to 4 packets
    interval = 1; // Default to 1 second interval

    if (argc < 2)
    {
        print_usage(argv[0]);
        return false;
    }

    target_ip = argv[argc - 1]; // Last argument is the target IP

    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
        {
            count = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
        {
            interval = atoi(argv[++i]);
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return false;
        }
    }

    return true;
}

void PingClient::print_usage(const char *prog_name)
{
    printf("Usage: %s [OPTIONS] DESTINATION\n", prog_name);
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("Options:\n");
    printf("  -c COUNT     Stop after sending COUNT packets (default: 4)\n");
    printf("  -i INTERVAL  Wait INTERVAL seconds between sending packets (default: 1)\n");
    printf("  -h           Display this help message\n");
    printf("\nDESTINATION must be an IP address.\n");
    printf("\nExample:\n");
    printf("  %s 192.168.0.1\n", prog_name);
    printf("  %s -c 10 -i 2 223.5.5.5\n", prog_name);
}