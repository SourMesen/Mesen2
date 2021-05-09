#pragma once
#include "NES/NesTypes.h"

struct NesPpuState
{
	PPUStatusFlags StatusFlags;
	int32_t Scanline;
	uint32_t Cycle;
	uint32_t FrameCount;
	uint32_t NmiScanline;
	uint32_t ScanlineCount;
	uint32_t SafeOamScanline;
	uint16_t BusAddress;
	uint8_t MemoryReadBuffer;
};
