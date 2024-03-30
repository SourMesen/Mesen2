#include "pch.h"
#include "PCE/PceNtscFilter.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/Video/GenericNtscFilter.h"

PceNtscFilter::PceNtscFilter(Emulator* emu) : PceDefaultVideoFilter(emu)
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
	FrameInfo frameInfo = PceDefaultVideoFilter::GetFrameInfo();
	if(_frameDivider == 0) {
		frameInfo.Width = SNES_NTSC_OUT_WIDTH(frameInfo.Width) / 2;
	} else {
		frameInfo.Width = SNES_NTSC_OUT_WIDTH(frameInfo.Width);
	}
	return frameInfo;
}

OverscanDimensions PceNtscFilter::GetOverscan()
{
	OverscanDimensions overscan = PceDefaultVideoFilter::GetOverscan();
	if(_frameDivider != 0) {
		overscan.Left = (uint32_t)(overscan.Left * 2 * 1.2);
		overscan.Right = (uint32_t)(overscan.Right * 2 * 1.2);
	}
	return overscan;
}

HudScaleFactors PceNtscFilter::GetScaleFactor()
{
	HudScaleFactors scaleFactors = PceDefaultVideoFilter::GetScaleFactor();
	scaleFactors.X *= (double)SNES_NTSC_OUT_WIDTH(256) / (256 * (_frameDivider == 0 ? 2 : 1));
	return scaleFactors;
}

void PceNtscFilter::OnBeforeApplyFilter()
{
	_pceConfig = _emu->GetSettings()->GetPcEngineConfig();

	if(GenericNtscFilter::NtscFilterOptionsChanged(_ntscSetup, _emu->GetSettings()->GetVideoConfig())) {
		GenericNtscFilter::InitNtscFilter(_ntscSetup, _emu->GetSettings()->GetVideoConfig());
		snes_ntsc_init(&_ntscData, &_ntscSetup);
	}
}

void PceNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	FrameInfo frameInfo = _frameInfo;
	FrameInfo baseFrameInfo = _baseFrameInfo;
	OverscanDimensions overscan = BaseVideoFilter::GetOverscan();

	constexpr uint32_t clockDividerOffset = PceConstants::MaxScreenWidth * PceConstants::ScreenHeight;
	uint32_t rowCount = PceConstants::ScreenHeight - overscan.Top - overscan.Bottom;
	uint32_t yOffset = overscan.Top * PceConstants::MaxScreenWidth;

	uint32_t frameWidth = _frameDivider ?
		(PceConstants::GetRowWidth(_frameDivider) - (overscan.Left + overscan.Right) * 4 / _frameDivider) :
		(_baseFrameInfo.Width - (overscan.Left + overscan.Right) * PceConstants::InternalResMultipler);

	uint8_t verticalScale = baseFrameInfo.Height / PceConstants::ScreenHeight;
	if(verticalScale != PceConstants::InternalResMultipler) {
		//Invalid data
		return;
	}

	//Convert RGB333 to RGB555 since this is what blargg's SNES NTSC filter expects
	for(uint32_t i = 0; i < rowCount; i++) {
		uint8_t clockDivider = _frameDivider ? _frameDivider : ppuOutputBuffer[clockDividerOffset + i + overscan.Top];
		uint32_t xOffset = PceConstants::GetLeftOverscan(clockDivider) + (overscan.Left * 4 / (clockDivider ? clockDivider : 4));
		uint32_t rowWidth = PceConstants::GetRowWidth(clockDivider);

		double ratio = _frameDivider ? 1.0 : ((double)rowWidth / baseFrameInfo.Width);
		uint32_t baseOffset = i * frameWidth;
		for(uint32_t j = 0; j < frameWidth; j++) {
			int pos = (int)(j * ratio);
			uint32_t color = _pceConfig.Palette[ppuOutputBuffer[i * PceConstants::MaxScreenWidth + pos + yOffset + xOffset] & 0x1FF];

			uint8_t r = (color >> 19) & 0x1F;
			uint8_t g = (color >> 11) & 0x1F;
			uint8_t b = (color >> 3) & 0x1F;

			_rgb555Buffer[baseOffset + j] = (b << 10) | (g << 5) | r;
		}
	}

	if(_frameDivider) {
		snes_ntsc_blit(&_ntscData, _rgb555Buffer, frameWidth, IsOddFrame() ? 0 : 1, frameWidth, rowCount, GetOutputBuffer(), frameInfo.Width * sizeof(uint32_t));
	} else {
		snes_ntsc_blit_hires(&_ntscData, _rgb555Buffer, frameWidth, IsOddFrame() ? 0 : 1, frameWidth, rowCount, _ntscBuffer, frameInfo.Width * sizeof(uint32_t));

		for(uint32_t i = 0; i < rowCount; i++) {
			uint32_t* src = _ntscBuffer + i * frameInfo.Width;
			for(uint32_t j = 0; j < verticalScale; j++) {
				uint32_t* dst = GetOutputBuffer() + (i * verticalScale + j) * frameInfo.Width;
				memcpy(dst, src, frameInfo.Width * sizeof(uint32_t));
			}
		}
	}
}
