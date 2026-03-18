#include "core/server/server.h"

#include "core/http/http_conn.h"
#include "core/net/socket_util.h"
#include "core/log/logger.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

Server::Server(const std::string& ip, int port, int threadPoolSize, int idleTimeoutMs,
               int maxConnectionsPerIp, const std::string& staticRoot)
    : ip_(ip),
      port_(port),
      listenFd_(-1),
      epoller_(1024),
      connectionGuard_(maxConnectionsPerIp),
      threadPool_(threadPoolSize),
      timerManager_(idleTimeoutMs),
      staticRoot_(staticRoot) {
    listenFd_ = net::SocketUtil::createListenFd(ip_, port_);
    epoller_.addFd(listenFd_, EPOLLIN);
}

Server::~Server() {
    for (int fd : clients_) {
        close(fd);
    }
    clients_.clear();

    if (listenFd_ >= 0) {
        close(listenFd_);
        listenFd_ = -1;
    }
}

void Server::start() {
    Logger::instance().info("AegisServer listening on " + ip_ + ":" + std::to_string(port_));

    while (true) {
        int ready = epoller_.wait(1000);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error("epoll_wait() failed");
        }

        for (int i = 0; i < ready; ++i) {
            epoll_event ev = epoller_.getEvent(i);
            int fd = ev.data.fd;

            if (fd == listenFd_) {
                handleListenEvent();
            } else if (ev.events & EPOLLIN) {
                handleClientRead(fd);
            } else if (ev.events & EPOLLOUT) {
                handleClientWrite(fd);
            } else {
                closeClient(fd);
            }
        }

        handleProcessCompletions();

        timerManager_.tick([this](int fd) {
            Logger::instance().info("timeout close fd=" + std::to_string(fd));
            closeClient(fd);
        });
    }
}

void Server::handleListenEvent() {
    while (true) {
        sockaddr_in clientAddr {};
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(listenFd_, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            std::cerr << "[WARN] accept() failed: " << std::strerror(errno) << std::endl;
            break;
        }

        char ipBuf[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipBuf, sizeof(ipBuf));
        std::string clientIp(ipBuf);

        if (!connectionGuard_.allow(clientIp)) {
            Logger::instance().warn("reject connection from " + clientIp +
                        ", current=" + std::to_string(connectionGuard_.currentConnections(clientIp)));
            metrics_.incRejectedConnections();
            sendRejectedResponse(clientFd);
            close(clientFd);
            continue;
        }

        try {
            net::SocketUtil::setNonBlocking(clientFd);
            epoller_.addFd(clientFd, EPOLLIN);
            clients_.insert(clientFd);

            auto conn = std::make_unique<HttpConn>();
            conn->init(clientFd, clientIp,staticRoot_);
            conns_[clientFd] = std::move(conn);

            connectionGuard_.onConnected(clientIp);
            metrics_.incActiveConnections();
            timerManager_.refresh(clientFd);

            Logger::instance().info("new connection fd=" + std::to_string(clientFd) +
                        " from " + clientIp +
                        ":" + std::to_string(ntohs(clientAddr.sin_port)));
        } catch (const std::exception& ex) {
            std::cerr << "[WARN] failed to register client fd=" << clientFd
                      << ", reason: " << ex.what() << std::endl;
            close(clientFd);
        }
    }
}

void Server::handleClientRead(int clientFd) {
    auto it = conns_.find(clientFd);
    if (it == conns_.end()) {
        closeClient(clientFd);
        return;
    }

    HttpConn& conn = *(it->second);

    if (conn.isProcessing()) {
        return;
    }

    if (!conn.read()) {
        closeClient(clientFd);
        return;
    }

    timerManager_.refresh(clientFd);

    conn.markProcessing(true);
    conn.markProcessDone(false);

    threadPool_.submit([this, clientFd]() {
        auto it2 = conns_.find(clientFd);
        if (it2 == conns_.end()) {
            return;
        }

        HttpConn& taskConn = *(it2->second);
        bool ok = taskConn.process(metrics_);
        if (!ok) {
            taskConn.markProcessDone(false);
            taskConn.markProcessing(false);
            return;
        }

        taskConn.markProcessDone(true);
        taskConn.markProcessing(false);
    });
}
void Server::handleClientWrite(int clientFd) {
    auto it = conns_.find(clientFd);
    if (it == conns_.end()) {
        closeClient(clientFd);
        return;
    }

    HttpConn& conn = *(it->second);

    if (!conn.write()) {
        closeClient(clientFd);
        return;
    }

    timerManager_.refresh(clientFd);

    if (conn.isKeepAlive()) {
        conn.reset();
        epoller_.modFd(clientFd, EPOLLIN);
    } else {
        closeClient(clientFd);
    }
}
void Server::closeClient(int clientFd) {
    if (clients_.find(clientFd) == clients_.end()) {
        return;
    }

    auto connIt = conns_.find(clientFd);
    if (connIt != conns_.end()) {
        connectionGuard_.onDisconnected(connIt->second->clientIp());
        metrics_.decActiveConnections();
    }

    timerManager_.remove(clientFd);

    try {
        epoller_.delFd(clientFd);
    } catch (...) {
    }

    close(clientFd);
    clients_.erase(clientFd);
    conns_.erase(clientFd);
}
void Server::sendRejectedResponse(int clientFd) {
    std::ifstream file(staticRoot_ +"/429.html");
    std::string body;

    if (file.is_open()) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
    } else {
        body = "<html><body><h1>429 Too Many Requests</h1></body></html>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 429 Too Many Requests\r\n";
    response << "Content-Type: text/html; charset=utf-8\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    send(clientFd, response.str().c_str(), response.str().size(), 0);
}
void Server::handleProcessCompletions() {
    for (auto& [fd, connPtr] : conns_) {
    HttpConn& conn = *connPtr;
    if (conn.isProcessDone() && !conn.isProcessing()) {
        conn.markProcessDone(false);
        try {
            epoller_.modFd(fd, EPOLLOUT);
        } catch (...) {
            closeClient(fd);
        }
    }
    }
}