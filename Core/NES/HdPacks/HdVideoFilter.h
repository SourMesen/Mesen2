#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "NES/HdPacks/HdNesPack.h"

class Emulator;
class NesConsole;
struct HdPackData;

class HdVideoFilter : public BaseVideoFilter
{
private:
	HdPackData* _hdData;
	unique_ptr<BaseHdNesPack> _hdNesPack = nullptr;

public:
	HdVideoFilter(NesConsole* console, Emulator* emu, HdPackData* hdData);
	virtual ~HdVideoFilter() = default;

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	OverscanDimensions GetOverscan() override;
};
