#pragma once
#include "stdafx.h"

class PceConstants
{
public:
	static constexpr uint32_t MasterClockRate = 21477270;

	static constexpr uint32_t MaxScreenWidth = 565;
	static constexpr uint32_t ScreenHeight = 242;

	static constexpr uint32_t ClockPerScanline = 1365;
	static constexpr uint32_t ScanlineCount = 263;
	
	//TODO TIMING - when does the active display start?
	//Some, but not all sources claim HDE/HDS/HSW has no impact on anything?
	static constexpr uint32_t DisplayStartHClock = 237;
};
