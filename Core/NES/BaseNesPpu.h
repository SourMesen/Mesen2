#pragma once 
#include "stdafx.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"

enum class ConsoleRegion;

class BaseNesPpu : public INesMemoryHandler, public ISerializable
{
protected:
	uint32_t _cycle;
	int32_t _scanline;
	uint32_t _frameCount;

public:
	virtual void Reset() = 0;
	virtual void Run(uint64_t runTo) = 0;

	uint32_t GetFrameCount() { return _frameCount; }
	uint32_t GetCurrentCycle() { return _cycle; }
	int32_t GetCurrentScanline() { return _scanline; }
	uint32_t GetFrameCycle() { return ((_scanline + 1) * 341) + _cycle; }

	virtual uint16_t* GetScreenBuffer(bool previousBuffer) = 0;
	virtual void SetRegion(ConsoleRegion region) = 0;

	virtual PpuModel GetPpuModel() = 0;
	virtual uint32_t GetPixelBrightness(uint8_t x, uint8_t y) = 0;

	virtual void GetMemoryRanges(MemoryRanges& ranges) override {}
	virtual uint8_t ReadRam(uint16_t addr) override { return 0; }
	virtual void WriteRam(uint16_t addr, uint8_t value) override {}

	virtual void Serialize(Serializer& s) override {}
};