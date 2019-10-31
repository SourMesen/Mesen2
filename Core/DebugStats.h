#pragma once
#include "stdafx.h"

class Console;

class DebugStats
{
private:
	double _frameDurations[60] = {};
	uint32_t _frameDurationIndex = 0;
	double _lastFrameMin = 9999;
	double _lastFrameMax = 0;

public:
	void DisplayStats(Console *console, double lastFrameTime);
};