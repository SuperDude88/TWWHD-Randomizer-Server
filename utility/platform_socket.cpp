#include "platform_socket.hpp"

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
}