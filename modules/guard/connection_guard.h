#pragma once

#include <string>
#include <unordered_map>

class ConnectionGuard {
public:
    explicit ConnectionGuard(int maxConnectionsPerIp = 3);

    bool allow(const std::string& ip);
    void onConnected(const std::string& ip);
    void onDisconnected(const std::string& ip);

    int currentConnections(const std::string& ip) const;

private:
    int maxConnectionsPerIp_;
    std::unordered_map<std::string, int> ipConnections_;
};