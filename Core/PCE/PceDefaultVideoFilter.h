#pragma once

#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"

class PceDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x200] = {};
	VideoConfig _videoConfig = {};

protected:
	static uint8_t To8Bit(uint8_t color)
	{
		return (color << 5) | (color << 2) | (color >> 1);
	}
	
	uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset)
	{
		return _calculatedPalette[ppuFrame[offset]];
	}

	FrameInfo GetFrameInfo() override
	{
		return BaseVideoFilter::GetFrameInfo();
	}

	void InitLookupTable()
	{
		VideoConfig config = _emu->GetSettings()->GetVideoConfig();

		InitConversionMatrix(config.Hue, config.Saturation);

		double y, i, q;
		for(int rgb333 = 0; rgb333 < 0x0200; rgb333++) {
			uint8_t g = To8Bit(rgb333 >> 6);
			uint8_t r = To8Bit((rgb333 >> 3) & 0x07);
			uint8_t b = To8Bit(rgb333 & 0x07);

			if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
				double redChannel = r / 255.0;
				double greenChannel = g / 255.0;
				double blueChannel = b / 255.0;

				//Apply brightness, contrast, hue & saturation
				RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
				y *= config.Contrast * 0.5f + 1;
				y += config.Brightness * 0.5f;
				YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

				r = (uint8_t)std::min(255, (int)(redChannel * 255));
				g = (uint8_t)std::min(255, (int)(greenChannel * 255));
				b = (uint8_t)std::min(255, (int)(blueChannel * 255));
				_calculatedPalette[rgb333] = 0xFF000000 | (r << 16) | (g << 8) | b;
			} else {
				_calculatedPalette[rgb333] = 0xFF000000 | (r << 16) | (g << 8) | b;
			}
		}

		_videoConfig = config;
	}

	void OnBeforeApplyFilter() override
	{
		VideoConfig config = _emu->GetSettings()->GetVideoConfig();

		if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness) {
			InitLookupTable();
		}
		_videoConfig = config;
	}

public:
	PceDefaultVideoFilter(Emulator* emu) : BaseVideoFilter(emu)
	{
		InitLookupTable();
	}

	void ApplyFilter(uint16_t* ppuOutputBuffer) override
	{
		uint32_t* out = GetOutputBuffer();
		FrameInfo frameInfo = _frameInfo;
		OverscanDimensions overscan = GetOverscan();

		uint32_t width = _baseFrameInfo.Width;
		uint32_t xOffset = overscan.Left;
		uint32_t yOffset = overscan.Top * width;

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
					out[i * frameInfo.Width + j] = GetPixel(ppuOutputBuffer, i * width + j + yOffset + xOffset);
				}
			}
		}
	}

	static uint32_t ToArgb(uint16_t rgb333)
	{
		uint8_t g = To8Bit(rgb333 >> 6);
		uint8_t r = To8Bit((rgb333 >> 3) & 0x07);
		uint8_t b = To8Bit(rgb333 & 0x07);

		return 0xFF000000 | (r << 16) | (g << 8) | b;
	}
};