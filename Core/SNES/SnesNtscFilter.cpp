#include "pch.h"
#include "SNES/SnesNtscFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/Video/GenericNtscFilter.h"

SnesNtscFilter::SnesNtscFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	memset(&_ntscData, 0, sizeof(_ntscData));
	_ntscSetup = { };
	snes_ntsc_init(&_ntscData, &_ntscSetup);
	_ntscBuffer = new uint32_t[SNES_NTSC_OUT_WIDTH(256) * 480];
}

FrameInfo SnesNtscFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();
	int widthDivider = _baseFrameInfo.Width == 512 ? 2 : 1;
	int heightMultiplier = _baseFrameInfo.Width == 512 ? 1 : 2;
	
	FrameInfo frameInfo;
	frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width / widthDivider) - overscan.Left - overscan.Right;
	frameInfo.Height = _baseFrameInfo.Height * heightMultiplier - overscan.Top - overscan.Bottom;
	return frameInfo;
}

OverscanDimensions SnesNtscFilter::GetOverscan()
{
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
	overscan.Top *= 2;
	overscan.Bottom *= 2;
	overscan.Left = (uint32_t)(overscan.Left * 2 * 1.2);
	overscan.Right = (uint32_t)(overscan.Right * 2 * 1.2);
	return overscan;
}

HudScaleFactors SnesNtscFilter::GetScaleFactor()
{
	return { (double)SNES_NTSC_OUT_WIDTH(256) / 256, 2 };
}

void SnesNtscFilter::OnBeforeApplyFilter()
{
	if(GenericNtscFilter::NtscFilterOptionsChanged(_ntscSetup, _emu->GetSettings()->GetVideoConfig())) {
		GenericNtscFilter::InitNtscFilter(_ntscSetup, _emu->GetSettings()->GetVideoConfig());
		snes_ntsc_init(&_ntscData, &_ntscSetup);
	}
}

void SnesNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	FrameInfo frameInfo = _frameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	bool useHighResOutput = _baseFrameInfo.Width == 512;
	uint32_t baseWidth = SNES_NTSC_OUT_WIDTH(256);
	uint32_t xOffset = overscan.Left;
	uint32_t yOffset = overscan.Top/2 * baseWidth;

	if(useHighResOutput) {
		snes_ntsc_blit_hires(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, IsOddFrame() ? 0 : 1, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, baseWidth * 4);
		
		for(uint32_t i = 0; i < frameInfo.Height; i++) {
			memcpy(GetOutputBuffer() + i * frameInfo.Width, _ntscBuffer + yOffset*2 + xOffset + i * baseWidth, frameInfo.Width * sizeof(uint32_t));
		}
	} else {
		snes_ntsc_blit(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, IsOddFrame() ? 0 : 1, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, baseWidth * 4);

		for(uint32_t i = 0; i < frameInfo.Height; i += 2) {
			memcpy(GetOutputBuffer() + i * frameInfo.Width, _ntscBuffer + yOffset + xOffset + i / 2 * baseWidth, frameInfo.Width * sizeof(uint32_t));
			memcpy(GetOutputBuffer() + (i + 1) * frameInfo.Width, _ntscBuffer + yOffset + xOffset + i / 2 * baseWidth, frameInfo.Width * sizeof(uint32_t));
		}
	}
}

SnesNtscFilter::~SnesNtscFilter()
{
	delete[] _ntscBuffer;
}