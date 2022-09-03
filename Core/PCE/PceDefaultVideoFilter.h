#pragma once
#include "stdafx.h"
#include "PCE/PceConstants.h"
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

	OverscanDimensions GetOverscan() override
	{
		OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
		overscan.Top *= PceConstants::InternalResMultipler;
		overscan.Bottom *= PceConstants::InternalResMultipler;
		overscan.Left *= PceConstants::InternalResMultipler;
		overscan.Right *= PceConstants::InternalResMultipler;
		return overscan;
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
		uint32_t* out = GetOutputBuffer();
		FrameInfo frameInfo = _frameInfo;
		FrameInfo baseFrameInfo = _baseFrameInfo;
		OverscanDimensions overscan = BaseVideoFilter::GetOverscan();

		uint32_t yOffset = overscan.Top * PceConstants::MaxScreenWidth;
		constexpr uint32_t clockDividerOffset = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;

		uint32_t rowCount = PceConstants::ScreenHeight - overscan.Top - overscan.Bottom;

		uint32_t verticalScale = baseFrameInfo.Height / PceConstants::ScreenHeight;
		
		if(verticalScale != PceConstants::InternalResMultipler) {
			//Invalid data
			return;
		}

		for(uint32_t i = 0; i < rowCount; i++) {
			uint8_t clockDivider = ppuOutputBuffer[clockDividerOffset + i + overscan.Top];
			uint32_t xOffset = PceConstants::GetLeftOverscan(clockDivider) + (overscan.Left * 4 / clockDivider);
			uint32_t rowWidth = PceConstants::GetRowWidth(clockDivider);

			//Interpolate row data across the whole screen
			double ratio = (double)rowWidth / baseFrameInfo.Width;

			uint32_t baseDstOffset = i * verticalScale * frameInfo.Width;
			uint32_t baseSrcOffset = i * PceConstants::MaxScreenWidth + yOffset + xOffset;
			for(uint32_t j = 0; j < frameInfo.Width; j++) {
				out[baseDstOffset + j] = GetPixel(ppuOutputBuffer, baseSrcOffset + (int)(j * ratio));
			}

			for(uint32_t j = 1; j < verticalScale; j++) {
				memcpy(out + baseDstOffset + (j * frameInfo.Width), out + baseDstOffset, frameInfo.Width * sizeof(uint32_t));
			}
		}
	}
};