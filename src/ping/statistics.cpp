/*
 * statistics.cpp
 * Implementation of ping statistics functionalities.
 */

#include "ping.h"

// Initialize statistics,
// for min and max time, use the max double value and 0.0
PingStatistics::PingStatistics()
    : packets_sent_(0), packets_received_(0), total_time_(0.0),
      min_time_(std::numeric_limits<double>::max()), max_time_(0.0) {}

void PingStatistics::add_response_time(double time_ms)
{
    total_time_ += time_ms;
    min_time_ = std::min(min_time_, time_ms);
    max_time_ = std::max(max_time_, time_ms);
}

void PingStatistics::print_statistics(const std::string& target_ip) const
{
    printf("\n--- %s ping statistics ---\n", target_ip.c_str());
    printf("%d packets transmitted, %d received, %.0f%% packet loss\n",
           packets_sent_, packets_received_, loss_rate());
    
    if (packets_received_ > 0) {
        printf("round-trip min/avg/max = %.3f/%.3f/%.3f ms\n",
               min_time_, average_time(), max_time_);
    }
}