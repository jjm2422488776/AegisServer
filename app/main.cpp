#include "core/config/config.h"
#include "core/log/logger.h"
#include "core/server/server.h"

#include <exception>

int main() {
    try {
        Config config;
        if (!config.loadFromFile("config/server.conf")) {
            Logger::instance().error("failed to load config/server.conf");
            return 1;
        }

        std::string ip = config.getString("listen_ip", "0.0.0.0");
        int port = config.getInt("listen_port", 8080);
        int threadPoolSize = config.getInt("thread_pool_size", 4);
        int idleTimeoutMs = config.getInt("idle_timeout_ms", 15000);
        int maxConnectionsPerIp = config.getInt("max_connections_per_ip", 3);
        std::string staticRoot = config.getString("static_root", "static");

        Logger::instance().info("config loaded successfully");

        Server server(ip, port, threadPoolSize, idleTimeoutMs, maxConnectionsPerIp, staticRoot);
        server.start();
    } catch (const std::exception& ex) {
        Logger::instance().error(std::string("fatal: ") + ex.what());
        return 1;
    }

    return 0;
}