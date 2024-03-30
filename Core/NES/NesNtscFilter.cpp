#include "pch.h"
#include "NES/NesNtscFilter.h"
#include "NES/NesDefaultVideoFilter.h"
#include "NES/NesConsole.h"
#include "NES/NesPpu.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/Video/GenericNtscFilter.h"

NesNtscFilter::NesNtscFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	memset(&_ntscData, 0, sizeof(_ntscData));
	_ntscSetup = { };
	nes_ntsc_init(&_ntscData, &_ntscSetup);
	_ntscBuffer = new uint32_t[NES_NTSC_OUT_WIDTH(256) * 240];
}

OverscanDimensions NesNtscFilter::GetOverscan()
{
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
	overscan.Top *= 2;
	overscan.Bottom *= 2;
	overscan.Left = (uint32_t)(overscan.Left * 2 * 1.2);
	overscan.Right = (uint32_t)(overscan.Right * 2 * 1.2);
	return overscan;
}

FrameInfo NesNtscFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();
	
	FrameInfo frameInfo;
	frameInfo.Width = NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) - overscan.Left - overscan.Right;
	frameInfo.Height = _baseFrameInfo.Height*2 - overscan.Top - overscan.Bottom;
	return frameInfo;
}

HudScaleFactors NesNtscFilter::GetScaleFactor()
{
	return { (double)NES_NTSC_OUT_WIDTH(256) / 256, 2 };
}

void NesNtscFilter::OnBeforeApplyFilter()
{
	NesConfig& nesCfg = _emu->GetSettings()->GetNesConfig();

	shared_ptr<IConsole> console = _emu->GetConsole();
	PpuModel model = ((NesConsole*)console.get())->GetPpu()->GetPpuModel();

	if(GenericNtscFilter::NtscFilterOptionsChanged(_ntscSetup, _emu->GetSettings()->GetVideoConfig()) || model != _ppuModel || memcmp(_nesConfig.UserPalette, nesCfg.UserPalette, sizeof(nesCfg.UserPalette)) != 0) {
		GenericNtscFilter::InitNtscFilter(_ntscSetup, _emu->GetSettings()->GetVideoConfig());

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
	FrameInfo frameInfo = _frameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	uint32_t baseWidth = NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
	uint32_t xOffset = overscan.Left;
	uint32_t yOffset = overscan.Top/2 * baseWidth;

	if(_nesConfig.EnablePalBorders && _emu->GetRegion() != ConsoleRegion::Ntsc) {
		NesDefaultVideoFilter::ApplyPalBorder(ppuOutputBuffer);
	}

	nes_ntsc_blit(&_ntscData, ppuOutputBuffer, _baseFrameInfo.Width, GetVideoPhase(), _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, NES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) * 4);

	for(uint32_t i = 0; i < frameInfo.Height; i+=2) {
		memcpy(GetOutputBuffer()+i*frameInfo.Width, _ntscBuffer + yOffset + xOffset + (i/2)*baseWidth, frameInfo.Width * sizeof(uint32_t));
		memcpy(GetOutputBuffer()+(i+1)*frameInfo.Width, _ntscBuffer + yOffset + xOffset + (i/2)*baseWidth, frameInfo.Width * sizeof(uint32_t));
	}
}

NesNtscFilter::~NesNtscFilter()
{
	delete[] _ntscBuffer;
}