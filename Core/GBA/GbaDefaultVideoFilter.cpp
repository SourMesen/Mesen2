#include "pch.h"
#include "GBA/GbaDefaultVideoFilter.h"
#include "GBA/GbaConsole.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/RewindManager.h"
#include "Shared/SettingTypes.h"
#include "Shared/ColorUtilities.h"

GbaDefaultVideoFilter::GbaDefaultVideoFilter(Emulator* emu, bool applyNtscFilter) : BaseVideoFilter(emu), _ntscFilter(emu)
{
	InitLookupTable();
	_applyNtscFilter = applyNtscFilter;
	_prevFrame = new uint16_t[GbaConstants::PixelCount];
	memset(_prevFrame, 0, GbaConstants::PixelCount * sizeof(uint16_t));
}

GbaDefaultVideoFilter::~GbaDefaultVideoFilter()
{
	delete[] _prevFrame;
}

FrameInfo GbaDefaultVideoFilter::GetFrameInfo()
{
	if(_applyNtscFilter) {
		FrameInfo frameInfo;
		frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
		frameInfo.Height = _baseFrameInfo.Height;
		return frameInfo;
	}
	return BaseVideoFilter::GetFrameInfo();
}

void GbaDefaultVideoFilter::InitLookupTable()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();

	InitConversionMatrix(config.Hue, config.Saturation);

	for(int rgb555 = 0; rgb555 < 0x8000; rgb555++) {
		uint8_t r = rgb555 & 0x1F;
		uint8_t g = (rgb555 >> 5) & 0x1F;
		uint8_t b = (rgb555 >> 10) & 0x1F;

		if(_gbaAdjustColors) {
			//Adjust colors to simulate LCD screen (uses the same formula as Ares)
			constexpr double lcdGamma = 4.0;
			constexpr double outGamma = 2.2;
			double lr = std::pow(r / 31.0, lcdGamma);
			double lg = std::pow(g / 31.0, lcdGamma);
			double lb = std::pow(b / 31.0, lcdGamma);
			r = (uint8_t)std::min<double>(std::pow((0 * lb + 50 * lg + 240 * lr) / 255, 1 / outGamma) * 0xFFFF / 280, 255);
			g = (uint8_t)std::min<double>(std::pow((30 * lb + 230 * lg + 10 * lr) / 255, 1 / outGamma) * 0xFFFF / 280, 255);
			b = (uint8_t)std::min<double>(std::pow((220 * lb + 10 * lg + 50 * lr) / 255, 1 / outGamma) * 0xFFFF / 280, 255);
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

void GbaDefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();
	GbaConfig gbaConfig = _emu->GetSettings()->GetGbaConfig();

	bool adjustColors = gbaConfig.GbaAdjustColors;
	if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness || _gbaAdjustColors != adjustColors) {
		_gbaAdjustColors = adjustColors;
		InitLookupTable();
	}
	bool blendFrames = gbaConfig.BlendFrames && !_emu->GetRewindManager()->IsRewinding() && !_emu->IsPaused();
	if(_blendFrames != blendFrames) {
		_blendFrames = blendFrames;
		memset(_prevFrame, 0, GbaConstants::PixelCount * sizeof(uint16_t));
	}

	_videoConfig = config;
}

void GbaDefaultVideoFilter::ApplyFilter(uint16_t* ppuOutputBuffer)
{
	uint32_t* out = GetOutputBuffer();

	for(uint32_t i = 0; i < GbaConstants::ScreenHeight; i++) {
		for(uint32_t j = 0; j < GbaConstants::ScreenWidth; j++) {
			out[i * GbaConstants::ScreenWidth + j] = GetPixel(ppuOutputBuffer, i * GbaConstants::ScreenWidth + j);
		}
	}

	if(_blendFrames) {
		std::copy(ppuOutputBuffer, ppuOutputBuffer + GbaConstants::PixelCount, _prevFrame);
	}

	if(_applyNtscFilter) {
		_ntscFilter.ApplyFilter(out, GbaConstants::ScreenWidth, GbaConstants::ScreenHeight, 0);
	}
}

uint32_t GbaDefaultVideoFilter::GetPixel(uint16_t* ppuFrame, uint32_t offset)
{
	if(_blendFrames) {
		return BlendPixels(_calculatedPalette[_prevFrame[offset] & 0x7FFF], _calculatedPalette[ppuFrame[offset] & 0x7FFF]);
	} else {
		return _calculatedPalette[ppuFrame[offset] & 0x7FFF];
	}
}

uint32_t GbaDefaultVideoFilter::BlendPixels(uint32_t a, uint32_t b)
{
	return ((((a) ^ (b)) & 0xfffefefeL) >> 1) + ((a) & (b));
}
