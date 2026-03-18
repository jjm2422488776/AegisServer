#include "core/metrics/metrics.h"

#include <sstream>

void Metrics::incActiveConnections() {
    ++activeConnections_;
}

void Metrics::decActiveConnections() {
    if (activeConnections_ > 0) {
        --activeConnections_;
    }
}

void Metrics::incTotalRequests() {
    ++totalRequests_;
}

void Metrics::incSuccessfulRequests() {
    ++successfulRequests_;
}

void Metrics::incNotFoundRequests() {
    ++notFoundRequests_;
}

void Metrics::incForbiddenRequests() {
    ++forbiddenRequests_;
}

void Metrics::incRejectedConnections() {
    ++rejectedConnections_;
}

void Metrics::incTooManyRequests() {
    ++tooManyRequests_;
}

void Metrics::incTimeoutClosed() {
    ++timeoutClosed_;
}

void Metrics::incKeepAliveHits() {
    ++keepAliveHits_;
}

void Metrics::addBytesSent(long long bytes) {
    bytesSent_ += bytes;
}

int Metrics::activeConnections() const {
    return activeConnections_.load();
}

long long Metrics::totalRequests() const {
    return totalRequests_.load();
}

long long Metrics::successfulRequests() const {
    return successfulRequests_.load();
}

long long Metrics::notFoundRequests() const {
    return notFoundRequests_.load();
}

long long Metrics::forbiddenRequests() const {
    return forbiddenRequests_.load();
}

long long Metrics::rejectedConnections() const {
    return rejectedConnections_.load();
}

long long Metrics::tooManyRequests() const {
    return tooManyRequests_.load();
}

long long Metrics::timeoutClosed() const {
    return timeoutClosed_.load();
}

long long Metrics::keepAliveHits() const {
    return keepAliveHits_.load();
}

long long Metrics::bytesSent() const {
    return bytesSent_.load();
}

std::string Metrics::toJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"active_connections\": " << activeConnections() << ",\n"
        << "  \"total_requests\": " << totalRequests() << ",\n"
        << "  \"successful_requests\": " << successfulRequests() << ",\n"
        << "  \"not_found_requests\": " << notFoundRequests() << ",\n"
        << "  \"forbidden_requests\": " << forbiddenRequests() << ",\n"
        << "  \"rejected_connections\": " << rejectedConnections() << ",\n"
        << "  \"too_many_requests\": " << tooManyRequests() << ",\n"
        << "  \"timeout_closed\": " << timeoutClosed() << ",\n"
        << "  \"keep_alive_hits\": " << keepAliveHits() << ",\n"
        << "  \"bytes_sent\": " << bytesSent() << "\n"
        << "}";
    return oss.str();
}