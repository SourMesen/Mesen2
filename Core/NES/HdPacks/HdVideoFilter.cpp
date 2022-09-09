#include "pch.h"
#include "NES/HdPacks/HdNesPack.h"
#include "NES/HdPacks/HdVideoFilter.h"
#include "NES/NesConsole.h"
#include "NES/NesConstants.h"
#include "Shared/Emulator.h"
#include "Shared/Video/BaseVideoFilter.h"

HdVideoFilter::HdVideoFilter(Emulator* emu, HdPackData* hdData) : BaseVideoFilter(emu)
{
	_hdData = hdData;
	_hdNesPack.reset(new HdNesPack(emu->GetSettings(), hdData));
}

FrameInfo HdVideoFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();
	uint32_t hdScale = _hdNesPack->GetScale();

	return {
		(NesConstants::ScreenWidth - overscan.Left - overscan.Right) * hdScale,
		(NesConstants::ScreenHeight - overscan.Top - overscan.Bottom) * hdScale
	};
}

OverscanDimensions HdVideoFilter::GetOverscan()
{
	if(_hdData->HasOverscanConfig) {
		return _hdData->Overscan;
	} else {
		return BaseVideoFilter::GetOverscan();
	}
}

void HdVideoFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	if(_frameData == nullptr) {
		//_frameData can be null when loading a save state
		return;
	}

	OverscanDimensions overscan = GetOverscan();
	_hdNesPack->Process((HdScreenInfo*)_frameData, GetOutputBuffer(), overscan);
}