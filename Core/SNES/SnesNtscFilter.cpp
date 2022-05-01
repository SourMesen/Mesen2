#include "stdafx.h"
#include "SNES/SnesNtscFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"

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
	frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width / widthDivider) - overscan.Left*2 - overscan.Right*2;
	frameInfo.Height = _baseFrameInfo.Height * heightMultiplier - overscan.Top*2 - overscan.Bottom*2;
	return frameInfo;
}

void SnesNtscFilter::OnBeforeApplyFilter()
{
	VideoConfig& cfg = _emu->GetSettings()->GetVideoConfig();

	if(_ntscSetup.hue != cfg.Hue / 100.0 || _ntscSetup.saturation != cfg.Saturation / 100.0 || _ntscSetup.brightness != cfg.Brightness / 100.0 || _ntscSetup.contrast != cfg.Contrast / 100.0 ||
		_ntscSetup.artifacts != cfg.NtscArtifacts || _ntscSetup.bleed != cfg.NtscBleed || _ntscSetup.fringing != cfg.NtscFringing || _ntscSetup.gamma != cfg.NtscGamma ||
		(_ntscSetup.merge_fields == 1) != cfg.NtscMergeFields || _ntscSetup.resolution != cfg.NtscResolution || _ntscSetup.sharpness != cfg.NtscSharpness) {
		_ntscSetup.hue = cfg.Hue;
		_ntscSetup.saturation = cfg.Saturation;
		_ntscSetup.brightness = cfg.Brightness;
		_ntscSetup.contrast = cfg.Contrast;

		_ntscSetup.artifacts = cfg.NtscArtifacts;
		_ntscSetup.bleed = cfg.NtscBleed;
		_ntscSetup.fringing = cfg.NtscFringing;
		_ntscSetup.gamma = cfg.NtscGamma;
		_ntscSetup.merge_fields = (int)cfg.NtscMergeFields;
		_ntscSetup.resolution = cfg.NtscResolution;
		_ntscSetup.sharpness = cfg.NtscSharpness;
		snes_ntsc_init(&_ntscData, &_ntscSetup);
	}
}

void SnesNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	FrameInfo frameInfo = _frameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	bool useHighResOutput = _baseFrameInfo.Width == 512;
	uint32_t baseWidth = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
	uint32_t xOffset = overscan.Left * 2;
	uint32_t yOffset = overscan.Top * 2 * baseWidth;

	if(useHighResOutput) {
		snes_ntsc_blit_hires(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, IsOddFrame() ? 0 : 1, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, baseWidth * 4);
	} else {
		snes_ntsc_blit(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, IsOddFrame() ? 0 : 1, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, baseWidth * 8);
	}

	for(uint32_t i = 0; i < frameInfo.Height; i+=2) {
		memcpy(GetOutputBuffer()+i*frameInfo.Width, _ntscBuffer + yOffset + xOffset + i*baseWidth, frameInfo.Width * sizeof(uint32_t));
		memcpy(GetOutputBuffer()+(i+1)*frameInfo.Width, _ntscBuffer + yOffset + xOffset + i*baseWidth, frameInfo.Width * sizeof(uint32_t));
	}
}

SnesNtscFilter::~SnesNtscFilter()
{
	delete[] _ntscBuffer;
}