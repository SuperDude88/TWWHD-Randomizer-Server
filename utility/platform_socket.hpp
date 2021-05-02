
#pragma once

#include "platform.hpp"

#ifdef PLATFORM_MSVC
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
	#define SocketType SOCKET
    #define SOCK_POLL WSAPoll
	#define SOCK_CLOSE closesocket
#else
	#include <netinet/in.h>
	#define SocketType int
	#define SOCK_POLL poll
	#define SOCK_CLOSE close
#endif

#if defined(PLATFORM_DKP)
	#include <nn/ac.h>
#endif

namespace Utility
{
	bool netInit();

	void netShutdown();

	bool isSocketInvalid(SocketType sock);
}