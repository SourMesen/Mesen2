#pragma once
#include "pch.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsTypes.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Shared/ColorUtilities.h"

class SmsDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000] = {};
	VideoConfig _videoConfig = {};
	SmsConsole* _console = nullptr;

protected:
	void OnBeforeApplyFilter() override
	{
		VideoConfig& config = _emu->GetSettings()->GetVideoConfig();
		if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness) {
			InitLookupTable();
		}
		_videoConfig = config;
	}

	void InitLookupTable()
	{
		VideoConfig config = _emu->GetSettings()->GetVideoConfig();

		InitConversionMatrix(config.Hue, config.Saturation);

		for(int rgb555 = 0; rgb555 < 0x8000; rgb555++) {
			uint8_t r = ColorUtilities::Convert5BitTo8Bit(rgb555 & 0x1F);
			uint8_t g = ColorUtilities::Convert5BitTo8Bit((rgb555 >> 5) & 0x1F);
			uint8_t b = ColorUtilities::Convert5BitTo8Bit((rgb555 >> 10) & 0x1F);

			if(config.Hue != 0 || config.Saturation != 0 || config.Brightness != 0 || config.Contrast != 0) {
				ApplyColorOptions(r, g, b, config.Brightness, config.Contrast);
				_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
			} else {
				_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
			}
		}

		_videoConfig = config;
	}

public:
	SmsDefaultVideoFilter(Emulator* emu, SmsConsole* console) : BaseVideoFilter(emu)
	{
		_console = console;
		InitLookupTable();
	}

	void ApplyFilter(uint16_t* ppuOutputBuffer) override
	{
		uint16_t* in = ppuOutputBuffer;
		uint32_t* out = GetOutputBuffer();
		
		OverscanDimensions overscan = GetOverscan();
		FrameInfo frame = _frameInfo;

		if(_console->GetModel() == SmsModel::GameGear) {
			int linesToSkip;
			switch(_console->GetVdp()->GetState().VisibleScanlineCount) {
				default: case 192: linesToSkip = 24; break;
				case 224: linesToSkip = 40; break;
				case 240: linesToSkip = 48; break;
			}

			for(uint32_t y = 0; y < frame.Height; y++) {
				for(uint32_t x = 0; x < frame.Width; x++) {
					out[(y * frame.Width) + x] = _calculatedPalette[in[(y + linesToSkip) * 256 + x + 48]];
				}
			}
		} else {
			uint32_t linesToSkip = _console->GetVdp()->GetViewportYOffset();
			uint32_t scanlineCount = _console->GetVdp()->GetState().VisibleScanlineCount;

			for(uint32_t y = 0; y < frame.Height; y++) {
				if(y + overscan.Top < linesToSkip || y > linesToSkip + scanlineCount - overscan.Top) {
					memset(out+y*frame.Width, 0, frame.Width * sizeof(uint32_t));
				} else {
					for(uint32_t x = 0; x < frame.Width; x++) {
						out[(y * frame.Width) + x] = _calculatedPalette[in[(y + overscan.Top - linesToSkip) * _baseFrameInfo.Width + x + overscan.Left]];
					}
				}
			}
		}
	}
};