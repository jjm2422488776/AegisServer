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

void Metrics::incNotFoundRequests() {
    ++notFoundRequests_;
}

void Metrics::incForbiddenRequests() {
    ++forbiddenRequests_;
}

void Metrics::incRejectedConnections() {
    ++rejectedConnections_;
}

int Metrics::activeConnections() const {
    return activeConnections_.load();
}

long long Metrics::totalRequests() const {
    return totalRequests_.load();
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

std::string Metrics::toJson() const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"active_connections\": " << activeConnections() << ",\n"
        << "  \"total_requests\": " << totalRequests() << ",\n"
        << "  \"not_found_requests\": " << notFoundRequests() << ",\n"
        << "  \"forbidden_requests\": " << forbiddenRequests() << ",\n"
        << "  \"rejected_connections\": " << rejectedConnections() << "\n"
        << "}";
    return oss.str();
}