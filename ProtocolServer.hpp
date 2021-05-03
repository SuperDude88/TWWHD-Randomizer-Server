
#pragma once

#include "utility/platform_socket.hpp"
#include <thread>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

class ProtocolServer
{
public:
    ProtocolServer(uint16_t port);

    bool initialize();

    bool start();

    bool stop();
private:
    struct ServerRequest
    {
        SocketType sock;
        std::string data;
    };
    uint16_t port;
    SocketType acceptSocket = -1;
    sockaddr_in serverAddr{};
    bool acceptingClients;
    bool receivingData;
    bool processingRequests;
    std::vector<SocketType> newSockets;
    std::mutex newSocketMut;
    std::unordered_map<SocketType, std::string> socketDataMap;
    std::vector<SocketType> removedSockets;
    std::queue<ServerRequest> serverRequests;
    std::mutex serverRequestsMut;
    std::condition_variable serverRequestsCV;
    std::thread acceptThread;
    std::thread receiveThread;
    std::thread processingThread;

    void acceptCallback();
    void receiveCallback();
    void processingCallback();
    void handleSocketRecvError(SocketType sock, int err);
    void handleSocketDisconnect(SocketType sock);
    void processSocketData();
};
