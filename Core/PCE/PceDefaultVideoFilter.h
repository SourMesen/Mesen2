#pragma once

#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"

class PceDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x400] = {};
	VideoConfig _videoConfig = {};
	PcEngineConfig _pceConfig = {};

protected:
	uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset)
	{
		return _calculatedPalette[ppuFrame[offset] & 0x3FF];
	}

	void InitLookupTable()
	{
		VideoConfig& config = _emu->GetSettings()->GetVideoConfig();
		PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();

		InitConversionMatrix(config.Hue, config.Saturation);

		double y, i, q;
		for(int rgb333 = 0; rgb333 < 0x0200; rgb333++) {
			uint32_t color = pceCfg.Palette[rgb333];
			uint8_t r = (color >> 16) & 0xFF;
			uint8_t g = (color >> 8) & 0xFF;
			uint8_t b = color & 0xFF;

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
			}

			//Convert RGB to grayscale color
			uint8_t grayscaleY = (uint8_t)std::clamp(0.299 * r + 0.587 * g + 0.114 * b, 0.0, 255.0);
			_calculatedPalette[rgb333 | 0x200] = 0xFF000000 | (grayscaleY << 16) | (grayscaleY << 8) | grayscaleY;

			//Regular RGB color
			_calculatedPalette[rgb333] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}

		_videoConfig = config;
	}

	void OnBeforeApplyFilter() override
	{
		VideoConfig& config = _emu->GetSettings()->GetVideoConfig();
		PcEngineConfig& pceConfig = _emu->GetSettings()->GetPcEngineConfig();

		bool optionsChanged = (
			_videoConfig.Hue != config.Hue ||
			_videoConfig.Saturation != config.Saturation ||
			_videoConfig.Contrast != config.Contrast ||
			_videoConfig.Brightness != config.Brightness ||
			memcmp(_pceConfig.Palette, pceConfig.Palette, sizeof(pceConfig.Palette)) != 0
		);

		if(optionsChanged) {
			InitLookupTable();
		}

		_videoConfig = config;
		_pceConfig = pceConfig;
	}

public:
	PceDefaultVideoFilter(Emulator* emu) : BaseVideoFilter(emu)
	{
		InitLookupTable();
	}

	void ApplyFilter(uint16_t* ppuOutputBuffer) override
	{
		uint8_t* rowVceClockDivider = (uint8_t*)_frameData;
		uint32_t* out = GetOutputBuffer();
		FrameInfo frameInfo = _frameInfo;
		OverscanDimensions overscan = GetOverscan();

		uint32_t yOffset = overscan.Top * PceConstants::MaxScreenWidth;

		for(uint32_t i = 0; i < frameInfo.Height / 2; i++) {
			uint32_t xOffset = PceConstants::GetLeftOverscan(rowVceClockDivider[i]);
			uint32_t rowWidth = PceConstants::GetRowWidth(rowVceClockDivider[i]);
			if(rowWidth == frameInfo.Width) {
				//512px output
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					uint32_t color = GetPixel(ppuOutputBuffer, i * PceConstants::MaxScreenWidth + j + yOffset + xOffset);
					out[i*2 * frameInfo.Width + j] = color;
					out[(i*2 + 1) * frameInfo.Width + j] = color;
				}
			} else {
				//Interpolate row data across the whole screen (or center, based on option?)
				double ratio = (double)rowWidth / frameInfo.Width;
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					double pos = (j * ratio);
					uint32_t color = GetPixel(ppuOutputBuffer, i * PceConstants::MaxScreenWidth + (int)pos + yOffset + xOffset);
					out[i * 2 * frameInfo.Width + j] = color;
					out[(i * 2 + 1) * frameInfo.Width + j] = color;
				}

				//TODO interpolation test
				/*
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					double pos = (j * ratio);
					double ratio = pos - (int)pos;

					uint32_t aColor = GetPixel(ppuOutputBuffer, i * PceConstants::MaxScreenWidth + (int)pos + yOffset + xOffset);
					uint32_t bColor = GetPixel(ppuOutputBuffer, i * PceConstants::MaxScreenWidth + (int)pos + 1 + yOffset + xOffset);

					uint8_t r = std::min<uint8_t>(255, (((aColor >> 16) & 0xFF) * (1.0 - ratio)) + (((bColor >> 16) & 0xFF) * ratio));
					uint8_t g = std::min<uint8_t>(255, (((aColor >> 8) & 0xFF) * (1.0 - ratio)) + (((bColor >> 8) & 0xFF) * ratio));
					uint8_t b = std::min<uint8_t>(255, ((aColor & 0xFF) * (1.0 - ratio)) + ((bColor & 0xFF) * ratio));

					uint32_t color = 0xFF000000 | (r << 16) | (g << 8) | b;
					out[i * 2 * frameInfo.Width + j] = color;
					out[(i * 2 + 1) * frameInfo.Width + j] = color;
				}
				*/
			}
		}
	}
};