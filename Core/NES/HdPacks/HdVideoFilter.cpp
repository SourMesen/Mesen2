#include "pch.h"
#include "NES/HdPacks/HdNesPack.h"
#include "NES/HdPacks/HdVideoFilter.h"
#include "NES/NesConsole.h"
#include "NES/NesConstants.h"
#include "Shared/Emulator.h"
#include "Shared/Video/BaseVideoFilter.h"

HdVideoFilter::HdVideoFilter(NesConsole* console, Emulator* emu, HdPackData* hdData) : BaseVideoFilter(emu)
{
	_hdData = hdData;
	switch(hdData->Scale) {
		case 1: _hdNesPack.reset(new HdNesPack<1>(console, emu->GetSettings(), hdData)); break;
		case 2: _hdNesPack.reset(new HdNesPack<2>(console, emu->GetSettings(), hdData)); break;
		case 3: _hdNesPack.reset(new HdNesPack<3>(console, emu->GetSettings(), hdData)); break;
		case 4: _hdNesPack.reset(new HdNesPack<4>(console, emu->GetSettings(), hdData)); break;
		case 5: _hdNesPack.reset(new HdNesPack<5>(console, emu->GetSettings(), hdData)); break;
		case 6: _hdNesPack.reset(new HdNesPack<6>(console, emu->GetSettings(), hdData)); break;
		case 7: _hdNesPack.reset(new HdNesPack<7>(console, emu->GetSettings(), hdData)); break;
		case 8: _hdNesPack.reset(new HdNesPack<8>(console, emu->GetSettings(), hdData)); break;
		case 9: _hdNesPack.reset(new HdNesPack<9>(console, emu->GetSettings(), hdData)); break;
		case 10: _hdNesPack.reset(new HdNesPack<10>(console, emu->GetSettings(), hdData)); break;
	}
	
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