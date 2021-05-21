
#include "utility/platform_socket.hpp"

#include <thread>
#include <chrono>
#include <cstring>

#include "ProtocolServer.hpp"

#ifdef PLATFORM_DKP
#include "utility/wiiutitles.hpp"
#include <vector>
#endif

#define SERVER_TCP_PORT 1234

int waitAndCleanup(const char* msg)
{
    Utility::platformLog("%s\n", msg);
    Utility::waitForPlatformStop();
    Utility::platformShutdown();
    Utility::netShutdown();
    return 0;
}

int
main(int argc, char** argv)
{
    Utility::netInit();
    if(!Utility::platformInit())
    {
        Utility::waitForPlatformStop();
        Utility::netShutdown();
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 0;
    }

    ProtocolServer server(SERVER_TCP_PORT);

    if (!server.initialize())
    {
        return waitAndCleanup("server.initialize() failed\n");
    }
    else
    {
        server.start();
    }   

    // wait until termination signal is received. In the case of wii u this is
    // the home button, otherwise, ctrl-c
    Utility::waitForPlatformStop();

    // if server never started, this does nothing
    server.stop();
    Utility::platformLog("server successfully stopped\n");

    Utility::platformShutdown();
    Utility::netShutdown();
    return 0;
}
