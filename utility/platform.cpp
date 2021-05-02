
#include "platform.hpp"
#include <thread>
#include <csignal>

#ifdef PLATFORM_DKP
	#include <whb/proc.h>
	#include <whb/log.h>
	#include <whb/log_console.h>
	#define PRINTF_BUFFER_LENGTH 2048
#endif 

static bool _platformIsRunning = true;

static void sigHandler(int signal)
{
	switch (signal)
	{
	case SIGINT:
#ifdef SIGBREAK
	case SIGBREAK:
#endif
		printf("ctrl+c\n");
		_platformIsRunning = false;
		break;
	}
}

namespace Utility
{
	void platformLog(const char* f, ...)
	{
#ifdef PLATFORM_DKP
		char buf[PRINTF_BUFFER_LENGTH];
#endif
		va_list args;
		va_start(args, f);
#ifdef PLATFORM_DKP
		vsnprintf(buf, PRINTF_BUFFER_LENGTH - 1, f, args);
		WHBLogWrite(buf);
		WHBLogConsoleDraw();
#else
		vprintf(f, args);
#endif
		va_end(args);
	}

	bool platformInit()
	{
#ifdef PLATFORM_DKP
		WHBProcInit();
		WHBLogConsoleInit();
#else
		signal(SIGINT, sigHandler);
		signal(SIGBREAK, sigHandler);
#endif
		return true;
	}

	bool platformIsRunning()
	{
#ifdef PLATFORM_DKP
		return WHBProcIsRunning();
#else
		return _platformIsRunning;
#endif
	}

	void waitForPlatformStop()
	{
		while (platformIsRunning())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void platformShutdown()
	{
#ifdef PLATFORM_DKP
		WHBLogConsoleFree();
		WHBProcShutdown();
#endif
	}
}