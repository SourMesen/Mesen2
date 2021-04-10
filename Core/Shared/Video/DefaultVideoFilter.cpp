#include "stdafx.h"
#include <algorithm>
#include "Shared/Video/DefaultVideoFilter.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"

DefaultVideoFilter::DefaultVideoFilter(shared_ptr<Emulator> emu) : BaseVideoFilter(emu)
{
	InitLookupTable();
	_prevFrame = new uint16_t[256 * 240];
	memset(_prevFrame, 0, 256 * 240 * sizeof(uint16_t));
}

void DefaultVideoFilter::InitLookupTable()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();

	InitConversionMatrix(config.Hue, config.Saturation);

	double y, i, q;
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
			r = To8Bit(r);
			g = To8Bit(g);
			b = To8Bit(b);
		}

		if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
			double redChannel = r / 255.0;
			double greenChannel = g / 255.0;
			double blueChannel = b / 255.0;

			//Apply brightness, contrast, hue & saturation
			RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
			y *= config.Contrast * 0.5f + 1;
			y += config.Brightness * 0.5f;
			YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

			int r = std::min(255, (int)(redChannel * 255));
			int g = std::min(255, (int)(greenChannel * 255));
			int b = std::min(255, (int)(blueChannel * 255));
			_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
		} else {
			_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}
	}

	_videoConfig = config;
}

void DefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _emu->GetSettings()->GetVideoConfig();
	GameboyConfig gbConfig = _emu->GetSettings()->GetGameboyConfig();

	ConsoleType consoleType = _emu->GetConsoleType();
	bool adjustColors = gbConfig.GbcAdjustColors && consoleType == ConsoleType::GameboyColor;
	if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness || _gbcAdjustColors != adjustColors) {
		_gbcAdjustColors = adjustColors;
		InitLookupTable();
	}
	_gbBlendFrames = gbConfig.BlendFrames && (consoleType == ConsoleType::Gameboy || consoleType == ConsoleType::GameboyColor);
	_videoConfig = config;
}

uint8_t DefaultVideoFilter::To8Bit(uint8_t color)
{
	return (color << 3) + (color >> 2);
}

uint32_t DefaultVideoFilter::ToArgb(uint16_t rgb555)
{
	uint8_t b = To8Bit(rgb555 >> 10);
	uint8_t g = To8Bit((rgb555 >> 5) & 0x1F);
	uint8_t r = To8Bit(rgb555 & 0x1F);

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void DefaultVideoFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	uint32_t *out = GetOutputBuffer();
	FrameInfo frameInfo = GetFrameInfo();
	OverscanDimensions overscan = GetOverscan();
	
	int overscanMultiplier = _baseFrameInfo.Width == 512 ? 2 : 1;
	uint32_t width = _baseFrameInfo.Width;
	uint32_t xOffset = overscan.Left * overscanMultiplier;
	uint32_t yOffset = overscan.Top * overscanMultiplier * width;

	uint8_t scanlineIntensity = (uint8_t)((1.0 - _emu->GetSettings()->GetVideoConfig().ScanlineIntensity) * 255);
	if(scanlineIntensity < 255) {
		for(uint32_t i = 0; i < frameInfo.Height; i++) {
			if(i & 0x01) {
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					*out = ApplyScanlineEffect(GetPixel(ppuOutputBuffer, i * width + j + yOffset + xOffset), scanlineIntensity);
					out++;
				}
			} else {
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					*out = GetPixel(ppuOutputBuffer, i * width + j + yOffset + xOffset);
					out++;
				}
			}
		}
	} else {
		for(uint32_t i = 0; i < frameInfo.Height; i++) {
			for(uint32_t j = 0; j < frameInfo.Width; j++) {
				out[i*frameInfo.Width+j] = GetPixel(ppuOutputBuffer, i * width + j + yOffset + xOffset);
			}
		}
	}

	if(_baseFrameInfo.Width == 512 && _videoConfig.BlendHighResolutionModes) {
		//Very basic blend effect for high resolution modes
		for(uint32_t i = 0; i < frameInfo.Height; i+=2) {
			for(uint32_t j = 0; j < frameInfo.Width; j+=2) {
				uint32_t &pixel1 = out[i*frameInfo.Width + j];
				uint32_t &pixel2 = out[i*frameInfo.Width + j + 1];
				uint32_t &pixel3 = out[(i+1)*frameInfo.Width + j];
				uint32_t &pixel4 = out[(i+1)*frameInfo.Width + j + 1];
				pixel1 = pixel2 = pixel3 = pixel4 = BlendPixels(BlendPixels(BlendPixels(pixel1, pixel2), pixel3), pixel4);
			}
		}
	}

	if(_gbBlendFrames) {
		std::copy(ppuOutputBuffer, ppuOutputBuffer + 256 * 240, _prevFrame);
	}
}

uint32_t DefaultVideoFilter::GetPixel(uint16_t* ppuFrame, uint32_t offset)
{
	if(_gbBlendFrames) {
		return BlendPixels(_calculatedPalette[_prevFrame[offset]], _calculatedPalette[ppuFrame[offset]]);
	} else {
		return _calculatedPalette[ppuFrame[offset]];
	}
}

uint32_t DefaultVideoFilter::BlendPixels(uint32_t a, uint32_t b)
{
	return ((((a) ^ (b)) & 0xfffefefeL) >> 1) + ((a) & (b));
}
