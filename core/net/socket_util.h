#pragma once

#include <string>

namespace net {

class SocketUtil {
public:
    static int createListenFd(const std::string& ip, int port, int backlog = 128);
    static void setNonBlocking(int fd);
    static void setReuseAddr(int fd);
};

}  // namespace net