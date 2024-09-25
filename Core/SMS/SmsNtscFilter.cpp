#include "pch.h"
#include "SMS/SmsNtscFilter.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsConsole.h"
#include "Shared/Video/GenericNtscFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"

SmsNtscFilter::SmsNtscFilter(Emulator* emu, SmsConsole* console) : BaseVideoFilter(emu)
{
	_console = console;
	_ntscData.reset(new sms_ntsc_t());
	_ntscSetup.reset(new sms_ntsc_setup_t());
	memset(_ntscData.get(), 0, sizeof(sms_ntsc_t));
	memset(_ntscSetup.get(), 0, sizeof(sms_ntsc_setup_t));
	sms_ntsc_init(_ntscData.get(), _ntscSetup.get());

	_snesNtscData.reset(new snes_ntsc_t());
	_snesNtscSetup.reset(new snes_ntsc_setup_t());
	memset(_snesNtscData.get(), 0, sizeof(snes_ntsc_t));
	memset(_snesNtscSetup.get(), 0, sizeof(snes_ntsc_setup_t));
	snes_ntsc_init(_snesNtscData.get(), _snesNtscSetup.get());

	uint32_t max = std::max(SMS_NTSC_OUT_WIDTH(256), SNES_NTSC_OUT_WIDTH(256));
	_ntscBuffer = new uint32_t[max * 240];
}

OverscanDimensions SmsNtscFilter::GetOverscan()
{
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
	overscan.Top *= 2;
	overscan.Bottom *= 2;
	overscan.Left = (uint32_t)(overscan.Left * 2 * 1.2);
	overscan.Right = (uint32_t)(overscan.Right * 2 * 1.2);
	return overscan;
}

FrameInfo SmsNtscFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();
	
	FrameInfo frameInfo;
	if(_console->GetModel() == SmsModel::GameGear) {
		frameInfo.Width = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) - overscan.Left - overscan.Right;
	} else {
		frameInfo.Width = SMS_NTSC_OUT_WIDTH(_baseFrameInfo.Width) - overscan.Left - overscan.Right;
	}
	frameInfo.Height = _baseFrameInfo.Height*2 - overscan.Top - overscan.Bottom;
	return frameInfo;
}

HudScaleFactors SmsNtscFilter::GetScaleFactor()
{
	if(_console->GetModel() == SmsModel::GameGear) {
		return { (double)SNES_NTSC_OUT_WIDTH(256) / 256, 2 };
	} else {
		return { (double)SMS_NTSC_OUT_WIDTH(256) / 256, 2 };
	}
}

void SmsNtscFilter::OnBeforeApplyFilter()
{
	if(GenericNtscFilter::NtscFilterOptionsChanged(*_ntscSetup.get(), _emu->GetSettings()->GetVideoConfig())) {
		GenericNtscFilter::InitNtscFilter(*_ntscSetup.get(), _emu->GetSettings()->GetVideoConfig());
		sms_ntsc_init(_ntscData.get(), _ntscSetup.get());

		GenericNtscFilter::InitNtscFilter(*_snesNtscSetup.get(), _emu->GetSettings()->GetVideoConfig());
		snes_ntsc_init(_snesNtscData.get(), _snesNtscSetup.get());
	}
}

void SmsNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	FrameInfo frame = _frameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	uint32_t xOffset = overscan.Left;
	uint32_t* out = GetOutputBuffer();
	
	uint32_t baseWidth;
	if(_console->GetModel() == SmsModel::GameGear) {
		baseWidth = SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
		snes_ntsc_blit(_snesNtscData.get(), ppuOutputBuffer, _baseFrameInfo.Width, 0, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, SNES_NTSC_OUT_WIDTH(_baseFrameInfo.Width) * 4);
	} else {
		baseWidth = SMS_NTSC_OUT_WIDTH(_baseFrameInfo.Width);
		sms_ntsc_blit(_ntscData.get(), ppuOutputBuffer, _baseFrameInfo.Width, _baseFrameInfo.Width, _baseFrameInfo.Height, _ntscBuffer, SMS_NTSC_OUT_WIDTH(_baseFrameInfo.Width) * 4);
	}

	uint32_t linesToSkip;
	uint32_t scanlineCount = _console->GetVdp()->GetState().VisibleScanlineCount;
	switch(scanlineCount) {
		default: case 192: linesToSkip = 24 * 2; break;
		case 224: linesToSkip = 8 * 2; break;
		case 240: linesToSkip = 0; break;
	}

	for(uint32_t y = 0; y < frame.Height; y += 2) {
		if(y + overscan.Top < linesToSkip || y > linesToSkip + scanlineCount * 2 - overscan.Top) {
			memset(out + y * frame.Width, 0, frame.Width * sizeof(uint32_t));
			memset(out + (y + 1) * frame.Width, 0, frame.Width * sizeof(uint32_t));
		} else {
			uint32_t* src = _ntscBuffer + xOffset + (y + overscan.Top - linesToSkip) / 2 * baseWidth;
			memcpy(out + y * frame.Width, src, frame.Width * sizeof(uint32_t));
			memcpy(out + (y + 1) * frame.Width, src, frame.Width * sizeof(uint32_t));
		}
	}
}

SmsNtscFilter::~SmsNtscFilter()
{
	delete[] _ntscBuffer;
}