#pragma once
#include "pch.h"

class PceConstants
{
public:
	static constexpr uint32_t MasterClockRate = 21477270;

	static constexpr uint32_t ClockPerScanline = 1365;
	static constexpr uint32_t ScanlineCount = 263;

	static constexpr uint32_t MaxScreenWidth = PceConstants::ClockPerScanline / 2;
	static constexpr uint32_t ScreenHeight = 242;

	static constexpr int RowOverscanSize = 18;
	static constexpr int InternalResMultipler = 4;

	static constexpr uint32_t InternalOutputWidth = (256 + PceConstants::RowOverscanSize * 2) * PceConstants::InternalResMultipler;
	static constexpr uint32_t InternalOutputHeight = PceConstants::ScreenHeight * PceConstants::InternalResMultipler;

	static constexpr uint32_t GetLeftOverscan(uint8_t vceClockDivider)
	{
		switch(vceClockDivider) {
			case 2:
				return 240 / 2 - (RowOverscanSize * 2);

			case 3:
				return 216 / 3 - (RowOverscanSize * 4 / 3);

			default:
			case 4:
				return 192 / 4 - RowOverscanSize;
		}
	}

	static constexpr uint32_t GetRowWidth(uint8_t vceClockDivider)
	{
		switch(vceClockDivider) {
			case 2: return 64 * 8 + (PceConstants::RowOverscanSize * 2 * 2);
			case 3: return 43 * 8 - 3 + (PceConstants::RowOverscanSize * 4 / 3 * 2);

			default:
			case 4: return 32 * 8 + PceConstants::RowOverscanSize * 2;
		}
	}
};
