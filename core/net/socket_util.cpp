#include "core/net/socket_util.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace net {

void SocketUtil::setNonBlocking(int fd) {
    int oldFlags = fcntl(fd, F_GETFL);
    if (oldFlags < 0) {
        throw std::runtime_error("fcntl(F_GETFL) failed");
    }

    if (fcntl(fd, F_SETFL, oldFlags | O_NONBLOCK) < 0) {
        throw std::runtime_error("fcntl(F_SETFL, O_NONBLOCK) failed");
    }
}

void SocketUtil::setReuseAddr(int fd) {
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }
}

int SocketUtil::createListenFd(const std::string& ip, int port, int backlog) {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        throw std::runtime_error("socket() failed");
    }

    try {
        setReuseAddr(listenFd);

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port));

        if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
            close(listenFd);
            throw std::runtime_error("inet_pton() failed for ip: " + ip);
        }

        if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            close(listenFd);
            throw std::runtime_error("bind() failed");
        }

        if (listen(listenFd, backlog) < 0) {
            close(listenFd);
            throw std::runtime_error("listen() failed");
        }

        setNonBlocking(listenFd);
    } catch (...) {
        close(listenFd);
        throw;
    }

    return listenFd;
}

}  // namespace net