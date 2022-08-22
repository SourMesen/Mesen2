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
	_ntscBuffer = new uint32_t[SNES_NTSC_OUT_WIDTH(PceConstants::InternalOutputWidth/2) * PceConstants::ScreenHeight];
	_rgb555Buffer = new uint16_t[PceConstants::InternalOutputWidth * PceConstants::ScreenHeight];
}

PceNtscFilter::~PceNtscFilter()
{
	delete[] _ntscBuffer;
	delete[] _rgb555Buffer;
}

FrameInfo PceNtscFilter::GetFrameInfo()
{
	FrameInfo frameInfo = BaseVideoFilter::GetFrameInfo();
	frameInfo.Width = SNES_NTSC_OUT_WIDTH(frameInfo.Width/2);
	return frameInfo;
}

OverscanDimensions PceNtscFilter::GetOverscan()
{
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();
	overscan.Top *= PceConstants::InternalResMultipler;
	overscan.Bottom *= PceConstants::InternalResMultipler;
	overscan.Left *= PceConstants::InternalResMultipler;
	overscan.Right *= PceConstants::InternalResMultipler;
	return overscan;
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
	FrameInfo frameInfo = _frameInfo;
	FrameInfo baseFrameInfo = _baseFrameInfo;
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();

	PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();

	constexpr uint32_t clockDividerOffset = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;
	uint32_t rowCount = PceConstants::ScreenHeight - overscan.Top - overscan.Bottom;
	uint32_t yOffset = overscan.Top * PceConstants::MaxScreenWidth;

	uint32_t frameWidth = _baseFrameInfo.Width - (overscan.Left + overscan.Right) * PceConstants::InternalResMultipler;

	uint8_t verticalScale = baseFrameInfo.Height / PceConstants::ScreenHeight;
	if(verticalScale != PceConstants::InternalResMultipler) {
		//Invalid data
		return;
	}

	//Convert RGB333 to RGB555 since this is what blargg's SNES NTSC filter expects
	for(uint32_t i = 0; i < rowCount; i++) {
		uint8_t clockDivider = ppuOutputBuffer[clockDividerOffset + i + overscan.Top];
		uint32_t xOffset = PceConstants::GetLeftOverscan(clockDivider) + (overscan.Left * 4 / clockDivider);
		uint32_t rowWidth = PceConstants::GetRowWidth(clockDivider);

		double ratio = (double)rowWidth / baseFrameInfo.Width;
		uint32_t baseOffset = i * frameWidth;
		for(uint32_t j = 0; j < frameWidth; j++) {
			int pos = (int)(j * ratio);
			uint32_t color = pceCfg.Palette[ppuOutputBuffer[i * PceConstants::MaxScreenWidth + pos + yOffset + xOffset] & 0x1FF];

			uint8_t r = (color >> 19) & 0x1F;
			uint8_t g = (color >> 11) & 0x1F;
			uint8_t b = (color >> 3) & 0x1F;

			_rgb555Buffer[baseOffset + j] = (b << 10) | (g << 5) | r;
		}
	}

	snes_ntsc_blit_hires(&_ntscData, _rgb555Buffer, frameWidth, IsOddFrame() ? 0 : 1, frameWidth, rowCount, _ntscBuffer, frameInfo.Width * sizeof(uint32_t));
	
	for(uint32_t i = 0; i < rowCount; i++) {
		uint32_t* src = _ntscBuffer + i * frameInfo.Width;
		for(uint32_t j = 0; j < verticalScale; j++) {
			uint32_t* dst = GetOutputBuffer() + (i * verticalScale + j) * frameInfo.Width;
			memcpy(dst, src, frameInfo.Width * sizeof(uint32_t));
		}
	}
}
