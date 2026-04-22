#include "epoll.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <sys/epoll.h>
#include <unistd.h>

Epoll::Epoll() {
    this->init();
}

Epoll::~Epoll() {
    try {
        this->stop();
    } catch (...) {
    }
}

void Epoll::init() {
    this->epollFd = epoll_create1(0);
    if (this->epollFd == -1) {
        throw std::runtime_error(std::string("epoll_create1 failed: ") + std::strerror(errno));
    }
    this->events.resize(128);
}

void Epoll::addEvent(int fd, int events) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
        throw std::runtime_error(std::string("epoll_ctl(ADD) failed: ") + std::strerror(errno));
    }
}
void Epoll::removeEvent(int fd) {
    if (epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        throw std::runtime_error(std::string("epoll_ctl(DEL) failed: ") + std::strerror(errno));
    }
}
void Epoll::start() {
    if (running) return;
    running = true;

    this->eventThread = std::thread([this]() {
        while (running) {
            int nfds = epoll_wait(this->epollFd, this->events.data(), static_cast<int>(this->events.size()), 1000);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                // epollFd 关闭后可能报错，直接退出循环
                break;
            }
            if (nfds == 0) continue;
            if (!eventCallback) continue;

            for (int i = 0; i < nfds; i++) {
                eventCallback(this->events[static_cast<size_t>(i)]);
            }
        }
    });
}
void Epoll::stop() {
    if (!running) return;
    running = false;
    if (eventThread.joinable()) eventThread.join();

    if (epollFd != -1) {
        ::close(epollFd);
        epollFd = -1;
    }
}
void Epoll::setEventCallback(std::function<void(const epoll_event&)> callback) {
    this->eventCallback = callback;
}