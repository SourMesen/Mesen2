#pragma once
#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"

class HdNesPack;
class Emulator;
struct HdPackData;

class HdVideoFilter : public BaseVideoFilter
{
private:
	HdPackData* _hdData;
	unique_ptr<HdNesPack> _hdNesPack = nullptr;

public:
	HdVideoFilter(Emulator* emu, HdPackData* hdData);

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	OverscanDimensions GetOverscan() override;
};
