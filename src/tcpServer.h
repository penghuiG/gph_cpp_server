#pragma once

#include <string>

#include <sys/epoll.h>

#include "accountReg.h"
#include "requestHandler.h"
#include "threadPoll.h"
#include "userSignIn.h"

class Epoll;

class TcpServer {
public:
    TcpServer(Epoll& epoll, int port);
    ~TcpServer();

    void start();
    void stop();

    void handleEventCallback(const epoll_event& event);

private:
    void acceptClient();
    void handleClient(int clientFd);
    void sendAll(int clientFd, const std::string& data);

    static int setNonBlocking(int fd);

private:
    Epoll& epoll;
    int port = 0;
    int serverSocket = -1;
    bool running = false;
    ThreadPool threadPool;

    AccountReg accountReg_;
    UserSignIn userSignIn_;
    RequestHandler requestHandler_;
};
