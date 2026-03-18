#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <queue>
#include <unordered_map>

class TimerManager {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    explicit TimerManager(int timeoutMs = 15000);

    void refresh(int fd);
    void remove(int fd);
    void tick(const std::function<void(int)>& onTimeout);

private:
    struct TimerNode {
        int fd;
        TimePoint expiresAt;

        bool operator<(const TimerNode& other) const {
            return expiresAt > other.expiresAt;
        }
    };

private:
    int timeoutMs_;
    std::priority_queue<TimerNode> heap_;
    std::unordered_map<int, TimePoint> latestExpire_;
};