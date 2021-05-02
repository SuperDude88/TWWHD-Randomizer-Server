#include "platform_socket.hpp"
#include <fcntl.h>

namespace Utility
{
	bool netInit()
	{
#ifdef PLATFORM_DKP
		nn::ac::ConfigIdNum configId;

		nn::ac::Initialize();
		nn::ac::GetStartupId(&configId);
		nn::ac::Connect(configId);
		return true;
#elif defined(PLATFORM_MSVC)
		WSADATA wsaData;
		int iResult;
		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			platformLog("WSAStartup failed: %d\n", iResult);
			return false;
		}
		return true;
#else
		return true;
#endif
	}

	void netShutdown()
	{
#ifdef PLATFORM_DKP
		nn::ac::Finalize();
#elif defined(PLATFORM_MSVC)
		WSACleanup();
#endif
	}

	bool isSocketInvalid(SocketType sock)
	{
#ifdef PLATFORM_MSVC
		return sock == INVALID_SOCKET;
#else
		return sock < 0;
#endif
	}

	bool setSocketBlockingFlag(SocketType sock, bool blocking)
	{
		if (isSocketInvalid(sock))
		{
			return false;
		}
#ifdef PLATFORM_MSVC
		unsigned long mode = blocking ? 0 : 1;
		return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
		int flags = fcntl(sock, F_GETFL, 0);
		int mask = 0;
		if (flags == -1) return false;
#ifdef PLATFORM_DKP
		mask = SO_NONBLOCK;
#else 
		mask = O_NONBLOCK;
#endif
		if (blocking) flags = flags & ~mask;
		else flags = flags | mask;
		return fcntl(sock, F_SETFL, flags) == 0;
#endif
	}
}