#include "core/net/epoller.h"

#include <stdexcept>
#include <unistd.h>

namespace net {

Epoller::Epoller(int maxEvents) : epollFd_(-1), events_(maxEvents) {
    epollFd_ = epoll_create1(0);
    if (epollFd_ < 0) {
        throw std::runtime_error("epoll_create1() failed");
    }
}

Epoller::~Epoller() {
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
}

void Epoller::addFd(int fd, uint32_t events) {
    epoll_event ev {};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl(ADD) failed");
    }
}

void Epoller::modFd(int fd, uint32_t events) {
    epoll_event ev {};
    ev.data.fd = fd;
    ev.events = events;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl(MOD) failed");
    }
}

void Epoller::delFd(int fd) {
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        throw std::runtime_error("epoll_ctl(DEL) failed");
    }
}

int Epoller::wait(int timeoutMs) {
    return epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
}

epoll_event Epoller::getEvent(size_t index) const {
    return events_.at(index);
}

}  // namespace net