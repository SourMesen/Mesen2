#include "stdafx.h"
#include "NES/NesNtscFilter.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "NES/NesPpu.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"

NesNtscFilter::NesNtscFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	memset(&_ntscData, 0, sizeof(_ntscData));
	_ntscSetup = { };
	nes_ntsc_init(&_ntscData, &_ntscSetup);
	_ntscBuffer = new uint32_t[NES_NTSC_OUT_WIDTH(256) * 240];
}

FrameInfo NesNtscFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();
	
	FrameInfo frameInfo;
	frameInfo.Width = NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) - overscan.Left*2 - overscan.Right*2;
	frameInfo.Height = _baseFrameInfo.Height*2 - overscan.Top*2 - overscan.Bottom*2;
	return frameInfo;
}

void NesNtscFilter::OnBeforeApplyFilter()
{
	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();
	NesConfig nesCfg = _emu->GetSettings()->GetNesConfig();
	PpuModel model = ((NesConsole*)_emu->GetConsole())->GetPpu()->GetPpuModel();

	if(_ntscSetup.hue != cfg.Hue || _ntscSetup.saturation != cfg.Saturation || _ntscSetup.brightness != cfg.Brightness || _ntscSetup.contrast != cfg.Contrast ||
		_ntscSetup.artifacts != cfg.NtscArtifacts || _ntscSetup.bleed != cfg.NtscBleed || _ntscSetup.fringing != cfg.NtscFringing || _ntscSetup.gamma != cfg.NtscGamma ||
		(_ntscSetup.merge_fields == 1) != cfg.NtscMergeFields || _ntscSetup.resolution != cfg.NtscResolution || _ntscSetup.sharpness != cfg.NtscSharpness || model != _ppuModel ||
		memcmp(_nesConfig.UserPalette, nesCfg.UserPalette, sizeof(nesCfg.UserPalette)) != 0) {

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

		uint32_t palette[512];
		NesDefaultVideoFilter::GetFullPalette(palette, nesCfg, model);
		for(int i = 0; i < 512; i++) {
			_palette[i * 3] = (palette[i] >> 16) & 0xFF;
			_palette[i * 3 + 1] = (palette[i] >> 8) & 0xFF;
			_palette[i * 3 + 2] = palette[i] & 0xFF;
		}
		_ntscSetup.palette = _palette;

		nes_ntsc_init(&_ntscData, &_ntscSetup);
	}

	_ppuModel = model;
	_nesConfig = nesCfg;
}

void NesNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	FrameInfo frameInfo = GetFrameInfo();
	OverscanDimensions overscan = GetOverscan();
	
	uint32_t baseWidth = NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
	uint32_t xOffset = overscan.Left * 2;
	uint32_t yOffset = overscan.Top * 2 * baseWidth;

	nes_ntsc_blit(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, IsOddFrame() ? 0 : 1, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) * 4);

	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();

	if(cfg.ScanlineIntensity == 0) {
		for(uint32_t i = 0; i < frameInfo.Height; i+=2) {
			memcpy(GetOutputBuffer()+i*frameInfo.Width, _ntscBuffer + yOffset + xOffset + (i/2)*baseWidth, frameInfo.Width * sizeof(uint32_t));
			memcpy(GetOutputBuffer()+(i+1)*frameInfo.Width, _ntscBuffer + yOffset + xOffset + (i/2)*baseWidth, frameInfo.Width * sizeof(uint32_t));
		}
	} else {
		uint8_t intensity = (uint8_t)((1.0 - cfg.ScanlineIntensity) * 255);
		for(uint32_t i = 0; i < frameInfo.Height; i++) {
			if(i & 0x01) {
				uint32_t *in = _ntscBuffer + yOffset + xOffset + (i/2) * baseWidth;
				uint32_t *out = GetOutputBuffer() + i * frameInfo.Width;
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					out[j] = ApplyScanlineEffect(in[j], intensity);
				}
			} else {
				memcpy(GetOutputBuffer()+i*frameInfo.Width, _ntscBuffer + yOffset + xOffset + (i/2)*baseWidth, frameInfo.Width * sizeof(uint32_t));
			}
		}
	}
}

NesNtscFilter::~NesNtscFilter()
{
	delete[] _ntscBuffer;
}