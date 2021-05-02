
#include "ProtocolServer.hpp"

#ifndef PLATFORM_MSVC
  #include <arpa/inet.h>
  #include <sys/types.h> 
  #include <sys/socket.h>
  #include <poll.h>
  #include <unistd.h>
  #include <errno.h>
  #include <fcntl.h>
#endif

#include <string.h>
#include <thread>
#include <chrono>

constexpr long POLL_TIMEOUT_MSEC = 100; // 100 ms

ProtocolServer::ProtocolServer(uint16_t port) 
    : port(port), acceptingClients(false), processingRequests(false)
{

}

bool ProtocolServer::initialize()
{
    acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (Utility::isSocketInvalid(acceptSocket))
    {
        return false;
    }
    // set socket to non-blocking
    if (!Utility::setSocketBlockingFlag(acceptSocket, false))
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

bool ProtocolServer::start()
{
    // TODO: check we are initialized
    acceptingClients = true;
    processingRequests = true;
    acceptThread = std::thread(&ProtocolServer::acceptCallback, this);
    processThread = std::thread(&ProtocolServer::processCallback, this);
    return true;
}

bool ProtocolServer::stop()
{
    acceptingClients = false;
    processingRequests = false;

    // wait for server threads to stop
    if (acceptThread.joinable())
    {
        acceptThread.join();
    }
    if (processThread.joinable())
    {
        processThread.join();
    }

    // close sockets
    if (acceptSocket != -1)
    {
        SOCK_CLOSE(acceptSocket);
    }

    for (const auto& sock : newSockets)
    {
        SOCK_CLOSE(sock);
    }
    newSockets.clear();
    for (const auto& sock : processingSockets)
    {
        SOCK_CLOSE(sock);
    }
    processingSockets.clear();
    acceptSocket = -1;
    return true;
}

void ProtocolServer::acceptCallback()
{
    SocketType clientSocket = INVALID_SOCKET;
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
        haveData = SOCK_POLL(pfds, 1, POLL_TIMEOUT_MSEC);
        if (haveData == 0)
        {
            if(haveData < 0)
            {
                Utility::platformLog("exited poll with errno %d\n", errno);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            continue;
        }
        if(pfds[0].revents & POLLIN)
        {
            std::lock_guard<std::mutex> guard(newSocketMut);
            bool validSocket = true;
            do
            {
                clientSocket = accept(acceptSocket, (struct sockaddr*)&clientAddr, &clientLen);
                validSocket = !Utility::isSocketInvalid(clientSocket);
                if (validSocket)
                {
                    newSockets.push_back(clientSocket);
                }
            } while (validSocket);
        }
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        Utility::platformLog("client connected from addr %s\n", clientIP);
    }
}

void ProtocolServer::processCallback()
{
    pollfd* pfds = nullptr;
    size_t prevSocketCount = 0, newSocketCount = 0, pfdsIndex = 0;
    int haveData;
    while (processingRequests)
    {
        {
            std::lock_guard<std::mutex> guard(newSocketMut);
            for (const auto& sockToAdd : newSockets)
            {
                processingSockets.push_back(sockToAdd);
            }
            newSockets.clear();
        }
        newSocketCount = processingSockets.size();
        if (newSocketCount != prevSocketCount)
        {
            if (pfds)
            {
                delete[] pfds;
            }
            pfds = new pollfd[newSocketCount];
        }
        if (!pfds) // extra null check
        {
            Utility::platformLog("waiting for clients...\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // POLL_TIMEOUT_MSEC
            continue;
        }
        // assemble poll file descriptor set
        pfdsIndex = 0;
        for (const auto sock : processingSockets)
        {
            pfds[pfdsIndex].fd = sock;
            pfds[pfdsIndex].events = POLLIN;
            pfdsIndex++;
        }
        // call poll on descriptor set
        haveData = SOCK_POLL(pfds, newSocketCount, POLL_TIMEOUT_MSEC);
        if (haveData == 0) continue; // timeout case
        else if (haveData < 0)
        {
            // handle error case by just logging for now
            Utility::platformLog("poll encountered an error %d\n", errno);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        pfdsIndex = 0;
        for (const auto& sock : processingSockets)
        {
            if (pfds[pfdsIndex].events & POLLIN)
            {
                Utility::platformLog("socket fd %d received data!\n", sock);
                // do processing on sock here
            }
            pfdsIndex++;
        }
    }
    if (pfds)
    {
        delete[] pfds;
    }
}
