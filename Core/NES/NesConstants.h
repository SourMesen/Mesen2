#pragma once
#include "pch.h"

#include "Shared/SettingTypes.h"

class NesConstants
{
public:
	static constexpr uint32_t ClockRateNtsc = 1789773;
	static constexpr uint32_t ClockRatePal = 1662607;
	static constexpr uint32_t ClockRateDendy = 1773448;

	static constexpr uint32_t CyclesPerLine = 341;

	static constexpr uint32_t ScreenWidth = 256;
	static constexpr uint32_t ScreenHeight = 240;
	static constexpr uint32_t ScreenPixelCount = ScreenWidth * ScreenHeight;

	static uint32_t GetClockRate(ConsoleRegion region)
	{
		switch(region) {
			default:
			case ConsoleRegion::Ntsc: return NesConstants::ClockRateNtsc; break;
			case ConsoleRegion::Pal: return NesConstants::ClockRatePal; break;
			case ConsoleRegion::Dendy: return NesConstants::ClockRateDendy; break;
		}
	}
};
