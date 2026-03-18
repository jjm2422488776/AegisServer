#pragma once

#include <sys/epoll.h>
#include <vector>

namespace net {

class Epoller {
public:
    explicit Epoller(int maxEvents = 1024);
    ~Epoller();

    void addFd(int fd, uint32_t events);
    void modFd(int fd, uint32_t events);
    void delFd(int fd);

    int wait(int timeoutMs);

    epoll_event getEvent(size_t index) const;

private:
    int epollFd_;
    std::vector<epoll_event> events_;
};

}  // namespace net