#pragma once
#include <memory>
#include "core/http/http_conn.h"
#include "core/metrics/metrics.h"
#include "core/net/epoller.h"
#include "core/threadpool/thread_pool.h"
#include "core/timer/timer_manager.h"
#include "modules/guard/connection_guard.h"
#include "core/config/config.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Server {
public:
    Server(const std::string& ip, int port, int threadPoolSize, int idleTimeoutMs,
       int maxConnectionsPerIp, const std::string& staticRoot);
    ~Server();

    void start();

private:
    void handleListenEvent();
    void handleClientRead(int clientFd);
    void handleClientWrite(int clientFd);
    void closeClient(int clientFd);
    void sendRejectedResponse(int clientFd);
    void handleProcessCompletions();

private:
    std::string staticRoot_;
    std::string ip_;
    int port_;
    int listenFd_;
    net::Epoller epoller_;
    std::unordered_set<int> clients_;
    std::unordered_map<int, std::unique_ptr<HttpConn>> conns_;
    Metrics metrics_;
    ConnectionGuard connectionGuard_;
    ThreadPool threadPool_;
    TimerManager timerManager_;
    std::mutex connMutex_;
};