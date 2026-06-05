#pragma once
#include "pch.h"

#ifdef _WIN32
#pragma comment(lib, "winmm.lib")
#endif

class PlatformUtilities
{
public:
	static void DisableScreensaver();
	static void EnableScreensaver();

	static void EnableHighResolutionTimer();
	static void RestoreTimerResolution();
};