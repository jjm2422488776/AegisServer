#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

class RateLimiter {
public:
    RateLimiter(int maxRequestsPerWindow = 20, int windowMs = 1000);

    bool allow(const std::string& ip);

private:
    struct WindowCounter {
        int count = 0;
        std::chrono::steady_clock::time_point windowStart;
    };

private:
    int maxRequestsPerWindow_;
    int windowMs_;
    std::unordered_map<std::string, WindowCounter> counters_;
};