#include "platform.hpp"

#ifdef PLATFORM_DKP
	#include <whb/proc.h>
	#include <whb/log.h>
	#include <whb/log_console.h>
	#define PRINTF_BUFFER_LENGTH 2048
#endif 

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
#endif
		return true;
	}

	bool platformIsRunning()
	{
#ifdef PLATFORM_DKP
		return WHBProcIsRunning();
#else
		return true;
#endif
	}

	void platformShutdown()
	{
#ifdef PLATFORM_DKP
		WHBLogConsoleFree();
		WHBProcShutdown();
#endif
	}
}