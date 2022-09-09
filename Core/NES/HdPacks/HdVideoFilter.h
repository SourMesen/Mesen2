#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "NES/HdPacks/HdNesPack.h"

class Emulator;
struct HdPackData;

class HdVideoFilter : public BaseVideoFilter
{
private:
	HdPackData* _hdData;
	unique_ptr<HdNesPack> _hdNesPack = nullptr;

public:
	HdVideoFilter(Emulator* emu, HdPackData* hdData);
	virtual ~HdVideoFilter() = default;

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	OverscanDimensions GetOverscan() override;
};
