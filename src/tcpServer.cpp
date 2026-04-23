#include "tcpServer.h"

#include "epoll.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer(Epoll& epoll, int port) : epoll(epoll), port(port) {}

TcpServer::~TcpServer() {
    try {
        stop();
    } catch (...) {
        // 析构里不抛异常
    }
}

int TcpServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void TcpServer::start() {
    if (running) return;

    serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error(std::string("setsockopt(SO_REUSEADDR) failed: ") + std::strerror(errno));
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        throw std::runtime_error(std::string("bind failed: ") + std::strerror(errno));
    }

    if (listen(serverSocket, 128) == -1) {
        throw std::runtime_error(std::string("listen failed: ") + std::strerror(errno));
    }

    if (setNonBlocking(serverSocket) == -1) {
        throw std::runtime_error(std::string("fcntl(O_NONBLOCK) failed: ") + std::strerror(errno));
    }

    epoll.addEvent(serverSocket, EPOLLIN);
    running = true;

    std::cout << "Server started on port " << port << std::endl;
}

void TcpServer::stop() {
    if (!running) return;
    running = false;

    if (serverSocket != -1) {
        try {
            epoll.removeEvent(serverSocket);
        } catch (...) {
        }
        ::close(serverSocket);
        serverSocket = -1;
    }
}

void TcpServer::handleEventCallback(const epoll_event& event) {
    const int fd = event.data.fd;

    if (fd == serverSocket) {//有新客户端连接
        acceptClient();//TODO:可以将这个函数加入线程池中?????????????????????????????,后面再研究
        return;
    }

    if (event.events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {//客户端断开连接
        epoll.removeEvent(fd);
        ::close(fd);
        return;
    }

    if (event.events & EPOLLIN) {//有数据可读
        handleClient(fd);
    }
}

void TcpServer::acceptClient() {
    while (true) {
        int clientFd = ::accept(serverSocket, nullptr, nullptr);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return; // 已经 accept 完了
            }
            throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
        }

        if (setNonBlocking(clientFd) == -1) {//设置非阻塞
            ::close(clientFd);
            continue;
        }
        ::write(clientFd, "Hello, this is server", 22);

        epoll.addEvent(clientFd, EPOLLIN | EPOLLRDHUP);//添加事件
    }
}

void TcpServer::handleClient(int clientFd) {
    char buf[4096];
    while (true) {
        ssize_t n = ::read(clientFd, buf, sizeof(buf));
        if (n == 0) {
            epoll.removeEvent(clientFd);
            ::close(clientFd);
            return;
        }
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            epoll.removeEvent(clientFd);
            ::close(clientFd);
            return;
        }

        // echo 回去
        ssize_t sent = 0;
        while (sent < n) {
            ssize_t m = ::write(clientFd, buf + sent, static_cast<size_t>(n - sent));
            if (m < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                epoll.removeEvent(clientFd);
                ::close(clientFd);
                return;
            }
            sent += m;
        }
    }
}
