#include <iostream>
#include "epoll.h"
#include "tcpServer.h"
#include "accountReg.h"
#include <thread>
#include <chrono>
#include "MysqlOperator.h"
#include "userSignIn.h"


int main() {
    accountRegTest();
    userSignInTest();
    // Epoll epoll;
    // TcpServer tcpServer(epoll, 8888);

    // epoll.setEventCallback([&](const epoll_event& event) { tcpServer.handleEventCallback(event); });
    // tcpServer.start();
    // epoll.start();
    // while (true) {
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }
    // return 0;

}