#pragma once
#include "pch.h"

struct TimingInfo
{
	double Fps;
	uint64_t MasterClock;
	uint32_t MasterClockRate;
	uint32_t FrameCount;
	
	uint32_t ScanlineCount;
	int32_t FirstScanline;
	uint32_t CycleCount;
};