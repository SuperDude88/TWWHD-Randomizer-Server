
#pragma once

#include <cstdarg>
#include <cstdio>

#ifdef _MSC_VER
	#define PLATFORM_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
	#define PLATFORM_GCC
#elif defined(__clang__)
	#define PLATFORM_CLANG
#else 
    #error UNKNOWN PLATFORM 
#endif
#ifdef DEVKITPRO
	#define PLATFORM_DKP
#endif

namespace Utility
{
	void platformLog(const char* f, ...);

	bool platformInit();

	bool platformIsRunning();

	void platformShutdown();
}
