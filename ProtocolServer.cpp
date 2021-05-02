
#include "ProtocolServer.hpp"

#ifndef PLATFORM_MSVC
  #include <arpa/inet.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <poll.h>
  #include <unistd.h>
  #include <errno.h>
#endif

#include <string.h>
#include <thread>
#include <chrono>

constexpr long ACCEPT_TIMEOUT_MSEC = 10000; // ~~100 ms~~ currently 10s for test

ProtocolServer::ProtocolServer(uint16_t port) : port(port), acceptingClients(false)
{

}

bool ProtocolServer::initialize()
{
    acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (Utility::isSocketInvalid(acceptSocket))
    {
        return false;
    }
    memset(reinterpret_cast<char*>(&serverAddr), '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if(bind(acceptSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        return false;
    }
    listen(acceptSocket, 5);
    return true;
}

void ProtocolServer::acceptCallback()
{
    SocketType clientSocket;
    int haveData;
    socklen_t clientLen;
    char clientIP[64];
    sockaddr_in clientAddr{};
    pollfd pfds[1]; // only need to hold one, but treat as an array for formality

    memset(clientIP, '\0', sizeof(clientIP));
    pfds[0].fd = acceptSocket;
    pfds[0].events = POLLIN;
    clientLen = sizeof(clientAddr);

    Utility::platformLog("starting accept loop\n");

    while(acceptingClients)
    {
        haveData = SOCK_POLL(pfds, 1, -1);
        if (haveData == 0)
        {
            Utility::platformLog("exited poll with haveData %d\n", haveData);
            if(haveData < 0)
            {
                Utility::platformLog("exited select with errno %d\n", errno);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            continue;
        }
        if(pfds[0].revents & POLLIN)
        {
            clientSocket = accept(acceptSocket, (struct sockaddr*)&clientAddr, &clientLen);
        }
        if(Utility::isSocketInvalid(clientSocket))
        {
            Utility::platformLog("client socket errno: %d\n", clientSocket);
            continue;
        }
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        Utility::platformLog("client connected from addr %s\n", clientIP);
    }
}

bool ProtocolServer::start()
{
    // TODO: check we are initialized
    acceptingClients = true;
    acceptThread = std::thread(&ProtocolServer::acceptCallback, this);
    return true;
}

bool ProtocolServer::stop()
{
    Utility::platformLog("shutting down server\n");
    acceptingClients = false;
    // TODO: something hanging on shutdown; unclear if thread or error caused by shutdown/close
    if(acceptThread.joinable())
    {
        acceptThread.join();
    }
    Utility::platformLog("accept thread exited\n");
    if (acceptSocket != -1)
    {
        SOCK_CLOSE(acceptSocket);
        Utility::platformLog("socket closed\n");
    }
    acceptSocket = -1;
    return true;
}
