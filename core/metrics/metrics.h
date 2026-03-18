#pragma once

#include <atomic>
#include <string>

class Metrics {
public:
    void incActiveConnections();
    void decActiveConnections();

    void incTotalRequests();
    void incSuccessfulRequests();
    void incNotFoundRequests();
    void incForbiddenRequests();
    void incRejectedConnections();
    void incTooManyRequests();
    void incTimeoutClosed();
    void incKeepAliveHits();

    void addBytesSent(long long bytes);

    int activeConnections() const;
    long long totalRequests() const;
    long long successfulRequests() const;
    long long notFoundRequests() const;
    long long forbiddenRequests() const;
    long long rejectedConnections() const;
    long long tooManyRequests() const;
    long long timeoutClosed() const;
    long long keepAliveHits() const;
    long long bytesSent() const;

    std::string toJson() const;

private:
    std::atomic<int> activeConnections_ {0};
    std::atomic<long long> totalRequests_ {0};
    std::atomic<long long> successfulRequests_ {0};
    std::atomic<long long> notFoundRequests_ {0};
    std::atomic<long long> forbiddenRequests_ {0};
    std::atomic<long long> rejectedConnections_ {0};
    std::atomic<long long> tooManyRequests_ {0};
    std::atomic<long long> timeoutClosed_ {0};
    std::atomic<long long> keepAliveHits_ {0};
    std::atomic<long long> bytesSent_ {0};
};