#pragma once

#include <functional>
#include <thread>
#include <vector>

#include <sys/epoll.h>

class Epoll {
public:
    Epoll();
    ~Epoll();
    void start();
    void stop();

    void addEvent(int fd, int events);
    void removeEvent(int fd);

    void setEventCallback(std::function<void(const epoll_event&)> callback);
private:
    void init();

private:
    int epollFd = -1;
    bool running = false;
    std::thread eventThread;
    std::function<void(const epoll_event&)> eventCallback;
    std::vector<epoll_event> events;
};