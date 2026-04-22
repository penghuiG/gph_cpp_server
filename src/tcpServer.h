#pragma once

#include <functional>
#include <string>

#include <sys/epoll.h>

class Epoll;

class TcpServer {
public:
    TcpServer(Epoll& epoll, int port);
    ~TcpServer();

    void start();
    void stop();

    // 由 Epoll 回调触发
    void handleEventCallback(const epoll_event& event);

private:
    void acceptClient();
    void handleClient(int clientFd);

    static int setNonBlocking(int fd);

private:
    Epoll& epoll;
    int port = 0;
    int serverSocket = -1;
    bool running = false;
};
