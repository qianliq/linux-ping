/*
 * main.cpp
 * Main entry point for the Ping project.
 *
 * WARNING: the code base on the system automatically add the IP header
 * and using the raw socket, so the code here is only for the ICMP header.
 */

#include "ping/ping.h"

// Ctrl+C then stop pinging
// touching the PingClient stop method
PingClient *g_ping_instance = nullptr;
void signal_handler(int signum)
{
    if (signum == SIGINT && g_ping_instance)
    {
        g_ping_instance->stop();
    }
}

int main(int argc, char *argv[])
{
    // configuration variables are not save until create the ping client
    std::string target_ip;
    int count, interval;

    // Parse command line arguments
    if (!PingClient::parse_arguments(argc, argv, target_ip, count, interval))
    {
        return 1;
    }

    // Create ping client
    PingClient ping_client(target_ip, count, interval);
    g_ping_instance = &ping_client; // Set global instance for signal handling

    // Initialize and run
    if (!ping_client.initialize())
    {
        return 1;
    }

    // Register signal handler for Ctrl+C
    signal(SIGINT, signal_handler);

    ping_client.run();

    return 0;
}