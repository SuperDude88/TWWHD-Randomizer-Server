
#pragma once

#include "utility/platform_socket.hpp"
#include <thread>
#include <vector>
#include <mutex>

class ProtocolServer
{
public:
    ProtocolServer(uint16_t port);

    bool initialize();

    bool start();

    bool stop();
private:
    uint16_t port;
    SocketType acceptSocket = -1;
    sockaddr_in serverAddr{};
    bool acceptingClients;
    bool processingRequests;
    std::vector<SocketType> newSockets;
    std::mutex newSocketMut;
    std::vector<SocketType> processingSockets;
    std::thread acceptThread;
    std::thread processThread;

    void acceptCallback();
    void processCallback();
};
