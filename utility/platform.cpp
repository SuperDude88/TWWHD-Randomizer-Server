
#include "platform.hpp"
#include <thread>
#include <csignal>
#include <mutex>

#ifdef PLATFORM_DKP
	#include <whb/proc.h>
	#include <whb/log.h>
	#include <whb/log_console.h>
	#include <coreinit/mcp.h>
	#include <coreinit/thread.h>
	#include "iosuhax.h"
	#include "iosuhax_devoptab.h"
	#define PRINTF_BUFFER_LENGTH 2048
	static int32_t mcpHookHandle = -1;
	static int32_t fsaHandle = -1;
	static int32_t iosuhaxHandle = -1;
	static bool systemMLCMounted = false;
#endif 

static bool _platformIsRunning = true;
static std::mutex printMut;

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

static bool initIOSUHax();
static void closeIosuhax();

namespace Utility
{
	void platformLog(const char* f, ...)
	{
#ifdef PLATFORM_DKP
		char buf[PRINTF_BUFFER_LENGTH];
#endif
		va_list args;
		va_start(args, f);
		std::unique_lock<std::mutex> lock(printMut);
#ifdef PLATFORM_DKP
		vsnprintf(buf, PRINTF_BUFFER_LENGTH - 1, f, args);
		
		WHBLogWrite(buf);
		WHBLogConsoleDraw();
#else
		vprintf(f, args);
#endif
		lock.unlock();
		va_end(args);
	}

	bool platformInit()
	{
#ifdef PLATFORM_DKP
		WHBProcInit();
		WHBLogConsoleInit();
		if(!initIOSUHax())
		{
			return false;
		}
		// if(mount_fs("storage_mlc01", getFSAHandle(), NULL, "/vol/storage_mlc01") == 0)
		// {
		// 	systemMLCMounted = true;
		// }
#else
		signal(SIGINT, sigHandler);
#ifdef SIGBREAK
		signal(SIGBREAK, sigHandler);
#endif
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
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}

	void platformShutdown()
	{
#ifdef PLATFORM_DKP
		WHBLogConsoleFree();
		WHBProcShutdown();
		// if (systemMLCMounted)
		// {
		// 	unmount_fs("storage_mlc01");
		// }
		closeIosuhax();
#endif
	}

	int32_t getFSAHandle()
	{
#ifdef PLATFORM_DKP
		return fsaHandle;
#else 
		return -1;
#endif
	}

	int32_t getMCPHandle()
	{
#ifdef PLATFORM_DKP
		return mcpHookHandle;
#else 
		return -1;
#endif
	}
}

#ifdef PLATFORM_DKP
void haxStartCallback(IOSError arg1, void *arg2) {
	Utility::platformLog("hasStartCallback arg1 = %d\n", (int)arg1);
}


bool initIOSUHax()
{
	Utility::platformLog("starting IOSUHax...\n");
	mcpHookHandle = MCP_Open();
	if (mcpHookHandle < 0)
	{
		Utility::platformLog("Unable to acquire mcp Hook handle\n");
		return false;
	}

	IOS_IoctlAsync(mcpHookHandle, 0x62, nullptr, 0, nullptr, 0, haxStartCallback, nullptr);
	OSSleepTicks(OSSecondsToTicks(2));
	iosuhaxHandle = IOSUHAX_Open("/dev/mcp");
	if (iosuhaxHandle < 0) {
		closeIosuhax();
        Utility::platformLog("Couldn't open iosuhax\n");
        return false;
    }

	fsaHandle = IOSUHAX_FSA_Open();
    if (fsaHandle < 0) {
		closeIosuhax();
        Utility::platformLog("Couldn't open iosuhax FSA!\n");
        return false;
    }
	Utility::platformLog(
		"mcpHookHandle: %d\niosuhaxHandle: %d\nfsaHandle: %d\n",
		mcpHookHandle, iosuhaxHandle, fsaHandle
	);
	return true;
}

void closeIosuhax() {
    if (fsaHandle > 0) IOSUHAX_FSA_Close(fsaHandle);
    if (iosuhaxHandle > 0) IOSUHAX_Close();
    if (mcpHookHandle > 0) MCP_Close(mcpHookHandle);
    OSSleepTicks(OSSecondsToTicks(1));
    mcpHookHandle = -1;
    fsaHandle = -1;
    iosuhaxHandle = -1;
}

#endif
