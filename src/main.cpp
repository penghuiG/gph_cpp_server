#include <iostream>
#include "epoll.h"
#include "tcpServer.h"
#include <thread>
#include <chrono>
int main() {
    Epoll epoll;
    TcpServer tcpServer(epoll, 8080);

    epoll.setEventCallback([&](const epoll_event& event) { tcpServer.handleEventCallback(event); });
    tcpServer.start();
    epoll.start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;

}