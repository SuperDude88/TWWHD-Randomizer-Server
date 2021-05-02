
#pragma once

#include "utility/platform_socket.hpp"
#include <thread>

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
    std::thread acceptThread;

    void acceptCallback();
};
