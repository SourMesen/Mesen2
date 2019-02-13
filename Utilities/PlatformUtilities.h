#pragma once
#include "stdafx.h"

class PlatformUtilities
{
private:
	static bool _highResTimerEnabled;

public:
	static void DisableScreensaver();
	static void EnableScreensaver();

	static void EnableHighResolutionTimer();
	static void RestoreTimerResolution();
};