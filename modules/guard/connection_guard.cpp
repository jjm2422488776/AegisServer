#include "modules/guard/connection_guard.h"

ConnectionGuard::ConnectionGuard(int maxConnectionsPerIp)
    : maxConnectionsPerIp_(maxConnectionsPerIp) {}

bool ConnectionGuard::allow(const std::string& ip) {
    auto it = ipConnections_.find(ip);
    int count = (it == ipConnections_.end()) ? 0 : it->second;
    return count < maxConnectionsPerIp_;
}

void ConnectionGuard::onConnected(const std::string& ip) {
    ++ipConnections_[ip];
}

void ConnectionGuard::onDisconnected(const std::string& ip) {
    auto it = ipConnections_.find(ip);
    if (it == ipConnections_.end()) {
        return;
    }

    --it->second;
    if (it->second <= 0) {
        ipConnections_.erase(it);
    }
}

int ConnectionGuard::currentConnections(const std::string& ip) const {
    auto it = ipConnections_.find(ip);
    if (it == ipConnections_.end()) {
        return 0;
    }
    return it->second;
}