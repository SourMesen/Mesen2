#pragma once
#include "stdafx.h"

struct TimingInfo
{
	double Fps;
	uint64_t MasterClock;
	uint32_t MasterClockRate;
	uint32_t FrameCount;
};