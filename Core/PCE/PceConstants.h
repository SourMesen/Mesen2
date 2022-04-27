#pragma once
#include "stdafx.h"

class PceConstants
{
public:
	static constexpr uint32_t MasterClockRate = 21477270;

	static constexpr uint32_t ClockPerScanline = 1365;
	static constexpr uint32_t ScanlineCount = 263;

	static constexpr uint32_t MaxScreenWidth = PceConstants::ClockPerScanline / 2;
	static constexpr uint32_t ScreenHeight = 242;

	static constexpr uint32_t GetLeftOverscan(uint8_t vceClockDivider)
	{
		switch(vceClockDivider) {
			case 2:
				return 112;

			default:
			case 3:
			case 4:
				return 5 * 8 * 4 / vceClockDivider;
		}
	}

	static constexpr uint32_t GetRowWidth(uint8_t vceClockDivider)
	{
		switch(vceClockDivider) {
			case 2: return 64 * 8;
			case 3: return 44 * 8;

			default:
			case 4: return 32 * 8;
		}
	}
};
