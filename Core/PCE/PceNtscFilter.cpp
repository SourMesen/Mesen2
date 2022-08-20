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
	_ntscBuffer = new uint32_t[SNES_NTSC_OUT_WIDTH(512) * PceConstants::ScreenHeight];
	_rgb555Buffer = new uint16_t[PceConstants::InternalOutputWidth * PceConstants::ScreenHeight];
}

PceNtscFilter::~PceNtscFilter()
{
	delete[] _ntscBuffer;
	delete[] _rgb555Buffer;
}

FrameInfo PceNtscFilter::GetFrameInfo()
{
	FrameInfo frameInfo;
	frameInfo.Width = SNES_NTSC_OUT_WIDTH(512);
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
	FrameInfo baseFrameInfo = _baseFrameInfo;
	OverscanDimensions overscan = GetOverscan();
	
	uint32_t baseWidth = SNES_NTSC_OUT_WIDTH(512);
	uint32_t xOffset = overscan.Left * 2;
	uint32_t yOffset = overscan.Top * 2 * baseWidth;
	PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();

	constexpr uint32_t clockDividerOffset = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;

	//Convert RGB333 to RGB555 since this is what blargg's SNES NTSC filter expects
	for(uint32_t i = 0; i < PceConstants::ScreenHeight; i++) {
		uint8_t clockDivider = ppuOutputBuffer[clockDividerOffset + i];
		uint32_t leftOverscan = PceConstants::GetLeftOverscan(clockDivider);
		uint32_t rowWidth = PceConstants::GetRowWidth(clockDivider);

		double ratio = (double)rowWidth / baseFrameInfo.Width;
		for(uint32_t j = 0; j < baseFrameInfo.Width; j++) {
			int pos = (int)(j * ratio);
			uint32_t color = pceCfg.Palette[ppuOutputBuffer[i * PceConstants::MaxScreenWidth + pos + leftOverscan] & 0x1FF];

			uint8_t r = (color >> 19) & 0x1F;
			uint8_t g = (color >> 11) & 0x1F;
			uint8_t b = (color >> 3) & 0x1F;

			_rgb555Buffer[i * baseFrameInfo.Width + j] = (b << 10) | (g << 5) | r;
		}
	}

	snes_ntsc_blit_hires(&_ntscData, _rgb555Buffer, baseFrameInfo.Width, IsOddFrame() ? 0 : 1, baseFrameInfo.Width, PceConstants::ScreenHeight, _ntscBuffer, baseWidth * 4);

	uint8_t verticalScale = frameInfo.Height / PceConstants::ScreenHeight;
	for(uint32_t i = 0; i < frameInfo.Height; i += verticalScale) {
		uint32_t* src = _ntscBuffer + yOffset + xOffset + i / verticalScale * baseWidth;
		uint32_t* dst = GetOutputBuffer() + i * baseWidth;
		int rowWidth = baseWidth * sizeof(uint32_t);
		for(int j = 0; j < verticalScale; j++) {
			memcpy(dst + (j * baseWidth), src, rowWidth);
		}
	}
}
