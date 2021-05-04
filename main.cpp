
#include "utility/platform_socket.hpp"

#include <thread>
#include <chrono>
#include <cstring>

#include "ProtocolServer.hpp"

#define SERVER_TCP_PORT 1234

int
main(int argc, char **argv)
{
   Utility::netInit();
   Utility::platformInit();

   ProtocolServer server(SERVER_TCP_PORT);

   if (!server.initialize())
   {
     Utility::platformLog("server.initialize() failed\n");
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
