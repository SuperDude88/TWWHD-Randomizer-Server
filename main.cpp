
#include "utility/platform.hpp"
#include "utility/platform_socket.hpp"

#include <thread>
#include <chrono>
#include <cstring>
#include <fstream>

#include "ProtocolServer.hpp"
#include "filetypes/wiiurpx.hpp"

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

    std::ifstream ifile(R"~(C:\workspace\temp\cking.rpx)~", std::ios::binary);
    int err;

    if (!ifile.is_open())
    {
        Utility::platformLog("unable to open cking.rpx\n");
    }
    else
    {
        if ((err = FileTypes::rpx_decompress(
            ifile,
            std::ofstream(R"~(C:\workspace\temp\cking.rpx.dec2)~", std::ios::binary)
        )))
        {
            Utility::platformLog("err: %d\n", err);
        }
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
