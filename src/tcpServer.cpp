#include "tcpServer.h"

#include "epoll.h"
#include "logger.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer(Epoll& epoll, int port)
    : epoll(epoll),
      port(port),
      threadPool(10),
      requestHandler_(accountReg_, userSignIn_) {}

TcpServer::~TcpServer() {
    try {
        stop();
    } catch (...) {
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

    LOG_INFO << "server started on port " << port;
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

    if (fd == serverSocket) {
        threadPool.submit([this]() {
            try {
                acceptClient();
            } catch (const std::exception& e) {
                LOG_ERROR << "accept error: " << e.what();
            }
        });
        return;
    }

    if (event.events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
        LOG_INFO << "client disconnected, fd=" << fd;
        epoll.removeEvent(fd);
        ::close(fd);
        return;
    }

    if (event.events & EPOLLIN) {
        threadPool.submit([this, fd]() {
            try {
                handleClient(fd);
            } catch (const std::exception& e) {
                LOG_ERROR << "client handler error, fd=" << fd << ", err=" << e.what();
                epoll.removeEvent(fd);
                ::close(fd);
            }
        });
    }
}

void TcpServer::acceptClient() {
    while (true) {
        int clientFd = ::accept(serverSocket, nullptr, nullptr);
        if (clientFd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
        }

        if (setNonBlocking(clientFd) == -1) {
            ::close(clientFd);
            continue;
        }

        sendAll(clientFd, "commands: REGISTER LOGIN LOGOUT UNREGISTER CHECK\n");
        LOG_INFO << "client connected, fd=" << clientFd;
        epoll.addEvent(clientFd, EPOLLIN | EPOLLRDHUP);
    }
}

void TcpServer::sendAll(int clientFd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::write(clientFd, data.data() + sent, data.size() - sent);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
        }
        sent += static_cast<size_t>(n);
    }
}

void TcpServer::handleClient(int clientFd) {
    char buf[4096];
    while (true) {
        ssize_t n = ::read(clientFd, buf, sizeof(buf) - 1);
        if (n == 0) {
            LOG_INFO << "client closed, fd=" << clientFd;
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

        buf[n] = '\0';
        LOG_DEBUG << "recv fd=" << clientFd << ", data=" << buf;
        const std::string response = requestHandler_.handle(buf);
        LOG_DEBUG << "send fd=" << clientFd << ", data=" << response;
        sendAll(clientFd, response);
    }
}
