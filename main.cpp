
#include "utility/platform_socket.hpp"

#include <thread>
#include <chrono>
#include <cstring>

#include "ProtocolServer.hpp"


int
hello_thread()
{
   int last_tm_sec = -1;
   uint32_t ip = 0;
   
   Utility::platformLog("Hello World from a std::thread!\n");

   ProtocolServer server(1234);
   if(!server.initialize())
   {
     Utility::platformLog("server.initialize() failed\n");
     return 1;
   }
   else
   {
      server.start();
   }

   server.stop();
   Utility::platformLog("Exiting... good bye.\n");
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   return 0;
}

int
main(int argc, char **argv)
{
   Utility::platformInit();
   Utility::netInit();

   ProtocolServer server(1234);

   if (!server.initialize())
   {
     Utility::platformLog("server.initialize() failed\n");
   }
   else
   {
     server.start();
   }

   while (Utility::platformIsRunning())
   {
     std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   // if server never started, this does nothing
   server.stop();

   Utility::netShutdown();
   Utility::platformShutdown();
   return 0;
}
