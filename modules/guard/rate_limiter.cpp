#include "modules/guard/rate_limiter.h"

RateLimiter::RateLimiter(int maxRequestsPerWindow, int windowMs)
    : maxRequestsPerWindow_(maxRequestsPerWindow),
      windowMs_(windowMs) {}

bool RateLimiter::allow(const std::string& ip) {
    auto now = std::chrono::steady_clock::now();
    auto& counter = counters_[ip];

    if (counter.count == 0) {
        counter.count = 1;
        counter.windowStart = now;
        return true;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - counter.windowStart).count();
    if (elapsed >= windowMs_) {
        counter.count = 1;
        counter.windowStart = now;
        return true;
    }

    if (counter.count >= maxRequestsPerWindow_) {
        return false;
    }

    ++counter.count;
    return true;
}