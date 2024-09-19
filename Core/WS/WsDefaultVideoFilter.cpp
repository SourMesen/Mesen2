#include "pch.h"
#include "WS/WsDefaultVideoFilter.h"
#include "WS/WsConsole.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Shared/ColorUtilities.h"
#include "Shared/RewindManager.h"

WsDefaultVideoFilter::WsDefaultVideoFilter(Emulator* emu, WsConsole* console, bool applyNtscFilter) : BaseVideoFilter(emu), _ntscFilter(emu)
{
	_emu = emu;
	_console = console;
	_applyNtscFilter = applyNtscFilter;
	_prevFrame = new uint16_t[WsConstants::MaxPixelCount];
	InitLookupTable();
}

WsDefaultVideoFilter::~WsDefaultVideoFilter()
{
	delete[] _prevFrame;
}

uint32_t WsDefaultVideoFilter::BlendPixels(uint32_t a, uint32_t b)
{
	return ((((a) ^ (b)) & 0xfffefefeL) >> 1) + ((a) & (b));
}

void WsDefaultVideoFilter::InitLookupTable()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();

	InitConversionMatrix(config.Hue, config.Saturation);

	for(int rgb444 = 0; rgb444 < 0x1000; rgb444++) {
		uint8_t r = (rgb444 >> 8) & 0xF;
		uint8_t g = (rgb444 >> 4) & 0xF;
		uint8_t b = rgb444 & 0xF;

		if(_adjustColors) {
			uint8_t r2 = r * 13 + g * 2 + b;
			uint8_t g2 = g * 12 + b * 4;
			uint8_t b2 = r * 3 + g * 2 + b * 11;
			r = r2;
			g = g2;
			b = b2;
		} else {
			r = ColorUtilities::Convert4BitTo8Bit(r);
			g = ColorUtilities::Convert4BitTo8Bit(g);
			b = ColorUtilities::Convert4BitTo8Bit(b);
		}

		if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
			ApplyColorOptions(r, g, b, config.Brightness, config.Contrast);
			_calculatedPalette[rgb444] = 0xFF000000 | (r << 16) | (g << 8) | b;
		} else {
			_calculatedPalette[rgb444] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}
	}

	_videoConfig = config;
}

FrameInfo WsDefaultVideoFilter::GetFrameInfo()
{
	if(_applyNtscFilter) {
		FrameInfo frameInfo;
		frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
		frameInfo.Height = _baseFrameInfo.Height;
		return frameInfo;
	}
	return BaseVideoFilter::GetFrameInfo();
}

void WsDefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();
	WsConfig wsConfig = _emu->GetSettings()->GetWsConfig();

	bool adjustColors = wsConfig.LcdAdjustColors;
	if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness || _adjustColors != adjustColors) {
		_adjustColors = adjustColors;
		InitLookupTable();
	}
	bool blendFrames = wsConfig.BlendFrames && !_emu->GetRewindManager()->IsRewinding() && !_emu->IsPaused();
	if(_blendFrames != blendFrames) {
		_blendFrames = blendFrames;
		memset(_prevFrame, 0, WsConstants::MaxPixelCount * sizeof(uint16_t));
	}
	_videoConfig = config;
}

void WsDefaultVideoFilter::ApplyFilter(uint16_t* ppuOutputBuffer)
{
	uint32_t* out = GetOutputBuffer();
	FrameInfo size = _baseFrameInfo;

	if(_blendFrames && _prevFrameSize.Width == size.Width && _prevFrameSize.Height == size.Height) {
		for(uint32_t i = 0, len = size.Height * size.Width; i < len; i++) {
			out[i] = BlendPixels(_calculatedPalette[_prevFrame[i]], _calculatedPalette[ppuOutputBuffer[i]]);
		}
	} else {
		for(uint32_t i = 0, len = size.Height * size.Width; i < len; i++) {
			out[i] = _calculatedPalette[ppuOutputBuffer[i]];
		}
	}

	if(_blendFrames) {
		std::copy(ppuOutputBuffer, ppuOutputBuffer + WsConstants::MaxPixelCount, _prevFrame);
		_prevFrameSize = size;
	}

	if(_applyNtscFilter) {
		_ntscFilter.ApplyFilter(out, size.Width, size.Height, 0);
	}
}
