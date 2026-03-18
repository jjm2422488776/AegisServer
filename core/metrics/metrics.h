#pragma once

#include <atomic>
#include <string>

class Metrics {
public:
    void incActiveConnections();
    void decActiveConnections();

    void incTotalRequests();
    void incNotFoundRequests();
    void incForbiddenRequests();
    void incRejectedConnections();

    int activeConnections() const;
    long long totalRequests() const;
    long long notFoundRequests() const;
    long long forbiddenRequests() const;
    long long rejectedConnections() const;

    std::string toJson() const;

private:
    std::atomic<int> activeConnections_ {0};
    std::atomic<long long> totalRequests_ {0};
    std::atomic<long long> notFoundRequests_ {0};
    std::atomic<long long> forbiddenRequests_ {0};
    std::atomic<long long> rejectedConnections_ {0};
};