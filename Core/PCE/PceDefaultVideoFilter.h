#pragma once
#include "pch.h"
#include "PCE/PceConstants.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"

class PceDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x400] = {};
	VideoConfig _videoConfig = {};

protected:
	PcEngineConfig _pceConfig = {};
	uint8_t _frameDivider = 0;
	FrameInfo _pceFrameSize = { 256, 242 };

	FrameInfo GetFrameInfo() override
	{
		if(_emu->GetRomInfo().Format == RomFormat::PceHes) {
			//Give a fixed 256x240 of space to HES player to match the other players
			FrameInfo frame;
			frame.Width = 256;
			frame.Height = 240;
			return frame;
		} else {
			UpdateFrameSize();
			return _pceFrameSize;
		}
	}

	void UpdateFrameSize()
	{
		if(!_pceConfig.ForceFixedResolution) {
			//Try to use an output resolution that matches the core's output (instead of forcing 4x scale)
			constexpr uint32_t clockDividerOffset = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;

			OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
			uint32_t rowCount = PceConstants::ScreenHeight - overscan.Top - overscan.Bottom;

			uint8_t frameDivider = 0;
			for(uint32_t i = 0; i < rowCount; i++) {
				uint8_t rowDivider = (uint8_t)_ppuOutputBuffer[clockDividerOffset + i + overscan.Top];
				if(frameDivider == 0) {
					frameDivider = rowDivider;
				} else if(frameDivider != rowDivider) {
					//Picture has multiple resolutions at once, use 4x scale
					frameDivider = 0;
					break;
				}
			}

			_frameDivider = frameDivider;
			
			//Refresh overscan left/right values after _frameDivider is updated
			overscan = BaseVideoFilter::GetOverscan();

			if(frameDivider) {
				FrameInfo size = {};
				size.Width = PceConstants::GetRowWidth(frameDivider) - (overscan.Left + overscan.Right) * 4 / frameDivider;
				size.Height = PceConstants::ScreenHeight - (overscan.Top + overscan.Bottom);
				_pceFrameSize = size;
			} else {
				_pceFrameSize = BaseVideoFilter::GetFrameInfo();
			}
		} else {
			//Always output at 4x scale (allows recording movies properly, etc.)
			_frameDivider = 0;
			_pceFrameSize = BaseVideoFilter::GetFrameInfo();
		}
	}

	uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset)
	{
		return _calculatedPalette[ppuFrame[offset] & 0x3FF];
	}

	HudScaleFactors GetScaleFactor() override
	{
		if(_emu->GetRomInfo().Format == RomFormat::PceHes) {
			return { 1, 1 };
		} else if(_frameDivider == 0) {
			return { PceConstants::InternalResMultipler, PceConstants::InternalResMultipler };
		} else {
			return { (double)4 / _frameDivider, 1 };
		}
	}

	OverscanDimensions GetOverscan() override
	{
		OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
		uint8_t frameDivider = _frameDivider;
		if(frameDivider == 0) {
			overscan.Top *= PceConstants::InternalResMultipler;
			overscan.Bottom *= PceConstants::InternalResMultipler;
			overscan.Left *= PceConstants::InternalResMultipler;
			overscan.Right *= PceConstants::InternalResMultipler;
		} else {
			overscan.Left *= 4.0 / frameDivider;
			overscan.Right *= 4.0 / frameDivider;
		}
		return overscan;
	}

	void InitLookupTable()
	{
		VideoConfig& config = _emu->GetSettings()->GetVideoConfig();
		PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();

		InitConversionMatrix(config.Hue, config.Saturation);

		for(int rgb333 = 0; rgb333 < 0x0200; rgb333++) {
			uint32_t color = pceCfg.Palette[rgb333];
			uint8_t r = (color >> 16) & 0xFF;
			uint8_t g = (color >> 8) & 0xFF;
			uint8_t b = color & 0xFF;

			if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
				ApplyColorOptions(r, g, b, config.Brightness, config.Contrast);
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
		if(_emu->GetRomInfo().Format == RomFormat::PceHes) {
			return;
		}

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

		if(_frameDivider != 0) {
			//Use dynamic resolution (changes based on the screen content)
			//Makes video filters work properly
			for(uint32_t i = 0; i < rowCount; i++) {
				uint32_t xOffset = PceConstants::GetLeftOverscan(_frameDivider) + (overscan.Left * 4 / _frameDivider);
				uint32_t baseDstOffset = i * frameInfo.Width;
				uint32_t baseSrcOffset = i * PceConstants::MaxScreenWidth + yOffset + xOffset;
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					out[baseDstOffset + j] = GetPixel(ppuOutputBuffer, baseSrcOffset + j);
				}
			}
		} else {
			//Always output at 4x scale
			for(uint32_t i = 0; i < rowCount; i++) {
				uint8_t clockDivider = ppuOutputBuffer[clockDividerOffset + i + overscan.Top];
				uint32_t xOffset = PceConstants::GetLeftOverscan(clockDivider) + (overscan.Left * 4 / (clockDivider ? clockDivider : 4));
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
	}
};