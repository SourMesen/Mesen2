#pragma once
#include "stdafx.h"

class Console;

class DebugStats
{
private:
	Console *_console;
	double _frameDurations[60] = {};
	uint32_t _frameDurationIndex = 0;
	double _lastFrameMin = 9999;
	double _lastFrameMax = 0;

public:
	DebugStats(Console *console);

	void DisplayStats(double lastFrameTime);
};