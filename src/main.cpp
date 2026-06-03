#include <iostream>
#include <thread>
#include <chrono>

#include "accountReg.h"
#include "dbOperator.h"
#include "epoll.h"
#include "logger.h"
#include "tcpServer.h"
#include "userSignIn.h"

int main() {
    Logger::instance().setMinLevel(LogLevel::Debug);
    Logger::instance().setLogFile("logs/server.log");
    LOG_INFO << "server starting";

    accountRegTest();
    userSignInTest();

    Epoll epoll;
    TcpServer tcpServer(epoll, 8888);

    epoll.setEventCallback([&](const epoll_event& event) { tcpServer.handleEventCallback(event); });
    tcpServer.start();
    epoll.start();

    LOG_INFO << "event loop running";
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
