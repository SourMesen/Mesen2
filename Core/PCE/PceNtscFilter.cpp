#include "stdafx.h"
#include "PCE/PceNtscFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"

PceNtscFilter::PceNtscFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	memset(&_ntscData, 0, sizeof(_ntscData));
	_ntscSetup = { };
	snes_ntsc_init(&_ntscData, &_ntscSetup);
	_ntscBuffer = new uint32_t[SNES_NTSC_OUT_WIDTH(256) * 242];
	_rgb555Buffer = new uint16_t[512 * 242];
}

PceNtscFilter::~PceNtscFilter()
{
	delete[] _ntscBuffer;
	delete[] _rgb555Buffer;
}

FrameInfo PceNtscFilter::GetFrameInfo()
{
	FrameInfo frameInfo;
	frameInfo.Width = SNES_NTSC_OUT_WIDTH(256);
	frameInfo.Height = _baseFrameInfo.Height;
	return frameInfo;
}

void PceNtscFilter::OnBeforeApplyFilter()
{
	if(NtscFilterOptionsChanged(_ntscSetup)) {		
		InitNtscFilter(_ntscSetup);
		snes_ntsc_init(&_ntscData, &_ntscSetup);
	}
}

void PceNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	//TODO overscan config
	FrameInfo frameInfo = _frameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	uint32_t baseWidth = SNES_NTSC_OUT_WIDTH(256);
	uint32_t xOffset = overscan.Left * 2;
	uint32_t yOffset = overscan.Top * 2 * baseWidth;
	PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();

	uint8_t* rowVceClockDivider = (uint8_t*)_frameData;

	//Convert RGB333 to RGB555 since this is what blargg's SNES NTSC filter expects
	for(uint32_t i = 0; i < frameInfo.Height / 2; i++) {
		uint32_t leftOverscan = PceConstants::GetLeftOverscan(rowVceClockDivider[i]);
		uint32_t rowWidth = PceConstants::GetRowWidth(rowVceClockDivider[i]);

		double ratio = (double)rowWidth / 512;
		for(uint32_t j = 0; j < 512; j++) {
			int pos = (int)(j * ratio);
			uint32_t color = pceCfg.Palette[ppuOutputBuffer[i * PceConstants::MaxScreenWidth + pos + leftOverscan] & 0x3FF];

			uint8_t r = (color >> 19) & 0x1F;
			uint8_t g = (color >> 11) & 0x1F;
			uint8_t b = (color >> 3) & 0x1F;

			_rgb555Buffer[i * 512 + j] = (b << 10) | (g << 5) | r;
		}
	}

	snes_ntsc_blit_hires(&_ntscData, _rgb555Buffer, 512, 0, 512, 242, _ntscBuffer, baseWidth * 4);

	for(uint32_t i = 0; i < frameInfo.Height; i+=2) {
		memcpy(GetOutputBuffer()+i*frameInfo.Width, _ntscBuffer + yOffset + xOffset + i/2*baseWidth, frameInfo.Width * sizeof(uint32_t));
		memcpy(GetOutputBuffer()+(i+1)*frameInfo.Width, _ntscBuffer + yOffset + xOffset + i/2*baseWidth, frameInfo.Width * sizeof(uint32_t));
	}
}
