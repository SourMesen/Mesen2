#include "pch.h"
#include "Gameboy/GbDefaultVideoFilter.h"
#include "Gameboy/GbConstants.h"
#include "Gameboy/Gameboy.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/RewindManager.h"
#include "Shared/SettingTypes.h"
#include "Shared/ColorUtilities.h"

GbDefaultVideoFilter::GbDefaultVideoFilter(Emulator* emu, bool applyNtscFilter) : BaseVideoFilter(emu), _ntscFilter(emu)
{
	InitLookupTable();
	_applyNtscFilter = applyNtscFilter;
	_prevFrame = new uint16_t[GbConstants::PixelCount];
	memset(_prevFrame, 0, GbConstants::PixelCount * sizeof(uint16_t));
}

GbDefaultVideoFilter::~GbDefaultVideoFilter()
{
	delete[] _prevFrame;
}

FrameInfo GbDefaultVideoFilter::GetFrameInfo()
{
	if(_emu->GetRomInfo().Format == RomFormat::Gbs) {
		//Give a fixed 256x240 of space to GBS player to match NES/SNES players
		FrameInfo frame;
		frame.Width = 256;
		frame.Height = 240;
		return frame;
	} else {
		if(_applyNtscFilter) {
			FrameInfo frameInfo;
			frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
			frameInfo.Height = _baseFrameInfo.Height;
			return frameInfo;
		}
		return BaseVideoFilter::GetFrameInfo();
	}
}

void GbDefaultVideoFilter::InitLookupTable()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();

	InitConversionMatrix(config.Hue, config.Saturation);

	for(int rgb555 = 0; rgb555 < 0x8000; rgb555++) {
		uint8_t r = rgb555 & 0x1F;
		uint8_t g = (rgb555 >> 5) & 0x1F;
		uint8_t b = (rgb555 >> 10) & 0x1F;
		if(_gbcAdjustColors) {
			uint8_t r2 = std::min(240, (r * 26 + g * 4 + b * 2) >> 2);
			uint8_t g2 = std::min(240, (g * 24 + b * 8) >> 2);
			uint8_t b2 = std::min(240, (r * 6 + g * 4 + b * 22) >> 2);
			r = r2;
			g = g2;
			b = b2;
		} else {
			r = ColorUtilities::Convert5BitTo8Bit(r);
			g = ColorUtilities::Convert5BitTo8Bit(g);
			b = ColorUtilities::Convert5BitTo8Bit(b);
		}

		if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
			ApplyColorOptions(r, g, b, config.Brightness, config.Contrast);
			_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
		} else {
			_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}
	}

	_videoConfig = config;
}

void GbDefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();
	GameboyConfig gbConfig = _emu->GetSettings()->GetGameboyConfig();

	bool adjustColors = gbConfig.GbcAdjustColors && ((Gameboy*)_emu->GetConsole().get())->IsCgb();
	if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness || _gbcAdjustColors != adjustColors) {
		_gbcAdjustColors = adjustColors;
		InitLookupTable();
	}
	bool blendFrames = gbConfig.BlendFrames && !_emu->GetRewindManager()->IsRewinding() && !_emu->IsPaused();
	if(_blendFrames != blendFrames) {
		_blendFrames = blendFrames;
		memset(_prevFrame, 0, GbConstants::PixelCount * sizeof(uint16_t));
	}
	_videoConfig = config;
}

void GbDefaultVideoFilter::ApplyFilter(uint16_t* ppuOutputBuffer)
{
	if(_emu->GetRomInfo().Format == RomFormat::Gbs) {
		return;
	}

	uint32_t* out = GetOutputBuffer();
	
	for(uint32_t i = 0; i < GbConstants::ScreenHeight; i++) {
		for(uint32_t j = 0; j < GbConstants::ScreenWidth; j++) {
			out[i * GbConstants::ScreenWidth + j] = GetPixel(ppuOutputBuffer, i * GbConstants::ScreenWidth + j);
		}
	}

	if(_blendFrames) {
		std::copy(ppuOutputBuffer, ppuOutputBuffer + GbConstants::PixelCount, _prevFrame);
	}

	if(_applyNtscFilter) {
		_ntscFilter.ApplyFilter(out, GbConstants::ScreenWidth, GbConstants::ScreenHeight, 0);
	}
}

uint32_t GbDefaultVideoFilter::GetPixel(uint16_t* ppuFrame, uint32_t offset)
{
	if(_blendFrames) {
		return BlendPixels(_calculatedPalette[_prevFrame[offset]], _calculatedPalette[ppuFrame[offset]]);
	} else {
		return _calculatedPalette[ppuFrame[offset]];
	}
}

uint32_t GbDefaultVideoFilter::BlendPixels(uint32_t a, uint32_t b)
{
	return ((((a) ^ (b)) & 0xfffefefeL) >> 1) + ((a) & (b));
}
