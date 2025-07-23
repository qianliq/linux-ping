/*
 * main.cpp
 * Main entry point for the Ping project.
 *
 * WARNING: the code base on the system automatically add the IP header
 * and using the raw socket, so the code here is only for the ICMP header.
 */

#include "ping/ping.h"

int main(int argc, char *argv[])
{
    // configuration variables are not save until create the ping client
    std::string target_ip;
    int count, interval;

    // Parse command line arguments
    if (!PingClient::parse_arguments(argc, argv, target_ip, count, interval)) {
        return 1;
    }

    // Create ping client
    PingClient ping_client(target_ip, count, interval);

    // Initialize and run
    if (!ping_client.initialize()) {
        return 1;
    }

    ping_client.run();

    return 0;
}