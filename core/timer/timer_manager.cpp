#include "core/timer/timer_manager.h"

TimerManager::TimerManager(int timeoutMs) : timeoutMs_(timeoutMs) {}

void TimerManager::refresh(int fd) {
    TimePoint expire = Clock::now() + std::chrono::milliseconds(timeoutMs_);
    latestExpire_[fd] = expire;
    heap_.push({fd, expire});
}

void TimerManager::remove(int fd) {
    latestExpire_.erase(fd);
}

void TimerManager::tick(const std::function<void(int)>& onTimeout) {
    TimePoint now = Clock::now();

    while (!heap_.empty()) {
        const TimerNode& node = heap_.top();

        auto it = latestExpire_.find(node.fd);
        if (it == latestExpire_.end()) {
            heap_.pop();
            continue;
        }

        if (it->second != node.expiresAt) {
            heap_.pop();
            continue;
        }

        if (node.expiresAt > now) {
            break;
        }

        int fd = node.fd;
        heap_.pop();
        latestExpire_.erase(fd);
        onTimeout(fd);
    }
}