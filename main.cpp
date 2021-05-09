
#include "utility/platform_socket.hpp"

#include <thread>
#include <chrono>
#include <cstring>

#include "ProtocolServer.hpp"

#ifdef PLATFORM_DKP
#include "utility/wiiutitles.hpp"
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
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
        Utility::platformLog("Failed in platform init\n");
        Utility::waitForPlatformStop();
        Utility::netShutdown();
        return 0;
    }

#ifdef PLATFORM_DKP
    std::vector<MCPTitleListType> rawTitles{};
    if (Utility::getRawTitles(rawTitles))
    {
        for (auto& title : rawTitles)
        {
            Utility::platformLog("titleId = %lld\n", title.titleId);
        }
        // std::vector<Utility::titleEntry> titles;
        // if (Utility::loadDetailedTitles(rawTitles, titles))
        // {
        //     for (const auto& title : titles)
        //     {
        //         Utility::platformLog("title: %s\n", title.shortTitle.c_str());
        //     }
        // }
        // else
        // {
        //     return waitAndCleanup("unable to load title info...");
        // }
    }
    else
    {
        return waitAndCleanup("unable to get raw titles...");
    }
#endif

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
