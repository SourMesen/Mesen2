//NTSC filter based on Bisqwit's code/algorithm
//As described here:
//http://forums.nesdev.com/viewtopic.php?p=172329
#include "pch.h"
#include <cmath>
#include "NES/BisqwitNtscFilter.h"
#include "NES/NesPpu.h"
#include "NES/NesConsole.h"
#include "NES/NesDefaultVideoFilter.h"
#include "Shared/EmuSettings.h"

BisqwitNtscFilter::BisqwitNtscFilter(Emulator* emu) : BaseVideoFilter(emu)
{
	_resDivider = 1;
	_stopThread = false;
	_workDone = false;

	// from https ://forums.nesdev.org/viewtopic.php?p=159266#p159266
	const double signalLumaLow[2][4] = {
		{ 0.228, 0.312, 0.552, 0.880 },
		{ 0.192, 0.256, 0.448, 0.712 }
	};
	const double signalLumaHigh[2][4] = {
		{ 0.616, 0.840, 1.100, 1.100 },
		{ 0.500, 0.676, 0.896, 0.896 }
	};
	double signal_blank = signalLumaLow[0][1];
	double signal_white = signalLumaHigh[0][3];

	//Precalculate the low and high signal chosen for each 64 base colors
	//with their respective attenuated values
	for(int h = 0; h <= 1; h++) {
		for(int i = 0; i <= 0x3F; i++) {

			double m = signalLumaLow[h][i / 0x10];
			double q = signalLumaHigh[h][i / 0x10];

			if((i & 0x0F) == 0x0D) {
				q = m;
			} else if((i & 0x0F) == 0) {
				m = q;
			} else if((i & 0x0F) >= 0x0E) {
				// colors $xE and $xF are not affected by emphasis
				// https://forums.nesdev.org/viewtopic.php?p=160669#p160669
				m = signalLumaLow[0][1];
				q = signalLumaLow[0][1];
			}

			_signalLow[(h ? 0x40 : 0) | i] = int8_t(std::floor(((m - signal_blank) / (signal_white - signal_blank)) * 100));
			_signalHigh[(h ? 0x40 : 0) | i] = int8_t(std::floor(((q - signal_blank) / (signal_white - signal_blank)) * 100));
		}
	}

	_extraThread = std::thread([=]() {
		//Worker thread to improve decode speed
		while(!_stopThread) {
			_waitWork.Wait();
			if(_stopThread) {
				break;
			}

			uint32_t* outputBuffer = GetOutputBuffer();
			FrameInfo frameInfo = _frameInfo;

			//Adjust outputbuffer to start at the middle of the picture
			int scale = 8 / _resDivider;
			outputBuffer += frameInfo.Width * ((120 - GetOverscan().Top) * scale);

			DecodeFrame(120, 239 - GetOverscan().Bottom, _ppuOutputBuffer, outputBuffer, (GetVideoPhase() * 4) + 327360);

			_workDone = true;
		}
	});
}

BisqwitNtscFilter::~BisqwitNtscFilter()
{
	_stopThread = true;
	_waitWork.Signal();
	_extraThread.join();
}

void BisqwitNtscFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	_ppuOutputBuffer = ppuOutputBuffer;

	if(_emu->GetSettings()->GetNesConfig().EnablePalBorders && _emu->GetRegion() != ConsoleRegion::Ntsc) {
		NesDefaultVideoFilter::ApplyPalBorder(ppuOutputBuffer);
	}

	_workDone = false;
	_waitWork.Signal();
	DecodeFrame(GetOverscan().Top, 120, ppuOutputBuffer, GetOutputBuffer(), (GetVideoPhase() * 4) + GetOverscan().Top*341*8);
	while(!_workDone) {}
}

FrameInfo BisqwitNtscFilter::GetFrameInfo()
{
	OverscanDimensions overscan = GetOverscan();

	VideoConfig& cfg = _emu->GetSettings()->GetVideoConfig();

	switch(cfg.NtscScale) {
		default:
		case NtscBisqwitFilterScale::_2x: _resDivider = 4; break;
		case NtscBisqwitFilterScale::_4x: _resDivider = 2; break;
		case NtscBisqwitFilterScale::_8x: _resDivider = 1; break;
	}

	int scale = 8 / _resDivider;
	FrameInfo frameInfo;
	frameInfo.Width = (_baseFrameInfo.Width - overscan.Left - overscan.Right) * scale;
	frameInfo.Height = (_baseFrameInfo.Height - overscan.Top - overscan.Bottom) * scale;
	return frameInfo;
}

HudScaleFactors BisqwitNtscFilter::GetScaleFactor()
{
	double scale = 8 / _resDivider;
	return { scale, scale };
}

void BisqwitNtscFilter::OnBeforeApplyFilter()
{
	VideoConfig& cfg = _emu->GetSettings()->GetVideoConfig();

	const double pi = std::atan(1.0) * 4;
	int contrast = (int)((cfg.Contrast + 1.0) * (cfg.Contrast + 1.0) * 167941);
	int saturation = (int)((cfg.Saturation + 1.0) * (cfg.Saturation + 1.0) * 144044);

	_brightness = (int)(cfg.Brightness * 750);

	for(int i = 0; i < 27; i++) {
		_sinetable[i] = (int8_t)(8 * std::sin(i * 2 * pi / 12 + cfg.Hue * pi));
	}

	_yWidth = std::max(1, (int)(12 + cfg.NtscYFilterLength * 24));
	_iWidth = std::max(12, (int)(12 + cfg.NtscIFilterLength * 24));
	_qWidth = std::max(12, (int)(12 + cfg.NtscQFilterLength * 24));

	_y = contrast / _yWidth;

	_ir = (int)(contrast * 1.994681e-6 * saturation / _iWidth);
	_qr = (int)(contrast * 9.915742e-7 * saturation / _qWidth);

	_ig = (int)(contrast * 9.151351e-8 * saturation / _iWidth);
	_qg = (int)(contrast * -6.334805e-7 * saturation / _qWidth);

	_ib = (int)(contrast * -1.012984e-6 * saturation / _iWidth);
	_qb = (int)(contrast * 1.667217e-6 * saturation / _qWidth);
}

void BisqwitNtscFilter::RecursiveBlend(int iterationCount, uint64_t *output, uint64_t *currentLine, uint64_t *nextLine, int pixelsPerCycle, bool verticalBlend)
{
	//Blend 2 pixels at once
	uint32_t width = _frameInfo.Width / 2;

	if(verticalBlend) {
		for(uint32_t x = 0; x < width; x++) {
			output[x] = ((((currentLine[x] ^ nextLine[x]) & 0xfefefefefefefefeL) >> 1) + (currentLine[x] & nextLine[x]));
		}
	} else {
		memcpy(output, currentLine, width * sizeof(uint64_t));
	}

	iterationCount /= 2;
	if(iterationCount > 0) {
		RecursiveBlend(iterationCount, output - width * iterationCount, currentLine, output, pixelsPerCycle, verticalBlend);
		RecursiveBlend(iterationCount, output + width * iterationCount, output, nextLine, pixelsPerCycle, verticalBlend);
	}
}

void BisqwitNtscFilter::GenerateNtscSignal(int8_t *ntscSignal, int &phase, int rowNumber)
{
	static constexpr uint16_t emphasisLut[8] = {
		//R: 0b000000111111, G: 0b001111110000, B: 0b111100000011
		0,              0b000000111111, 0b001111110000, 0b001111111111,
		0b111100000011, 0b111100111111, 0b111111110011, 0b111111111111
	};

	for(int x = 0; x < 256; x++) {
		uint16_t ppuData = _ppuOutputBuffer[(rowNumber << 8) | x];
		
		uint16_t pixelColor = ppuData & 0x3F;
		uint8_t emphasis = ppuData >> 6;
		uint8_t hue = ppuData & 0x0F;

		uint16_t emphasisWave = 0;
		if(emphasis) {
			// phase shift 12-bit waveform relative to pixel hue
			emphasisWave = ((emphasisLut[emphasis] >> (hue % 12)) | (emphasisLut[emphasis] << (12 - (hue % 12)))) & 0xFFFF;
		}

		uint16_t phaseBitmask = _bitmaskLut[std::abs(phase - hue) % 12];
		for(int j = 0; j < _signalsPerPixel; j++) {
			phaseBitmask <<= 1;
			
			uint8_t color = pixelColor | ((phaseBitmask & emphasisWave) ? 0x40 : 0);
			int8_t voltage = _signalHigh[color];

			// 12 phases done, wrap back to beginning
			if(phaseBitmask >= (1 << 12)) {
				phaseBitmask = 1;
			} else if(phaseBitmask >= (1 << 6)) {
				// 6 out of 12 cycles
				voltage = _signalLow[color];
			}
			ntscSignal[(x << 3) | j] = voltage;
		}

		phase += _signalsPerPixel;
	}
	phase += (341 - 256) * _signalsPerPixel;
}

void BisqwitNtscFilter::DecodeFrame(int startRow, int endRow, uint16_t *ppuOutputBuffer, uint32_t* outputBuffer, int startPhase)
{
	int pixelsPerCycle = 8 / _resDivider;
	int phase = startPhase;
	constexpr int lineWidth = 256;
	int8_t rowSignal[lineWidth * _signalsPerPixel];
	uint32_t rowPixelGap = _frameInfo.Width * pixelsPerCycle;
	
	uint32_t* orgBuffer = outputBuffer;

	for(int y = startRow; y <= endRow; y++) {
		int startCycle = phase % 12;
		
		//Convert the PPU's output to an NTSC signal
		GenerateNtscSignal(rowSignal, phase, y);

		//Convert the NTSC signal to RGB
		NtscDecodeLine(lineWidth * _signalsPerPixel, rowSignal, outputBuffer, (startCycle + 7) % 12);

		outputBuffer += rowPixelGap;
	}

	//Generate the missing vertical lines
	outputBuffer = orgBuffer;
	int lastRow = 239 - GetOverscan().Bottom;
	bool verticalBlend = false; //_emu->GetSettings()->GetVideoConfig();
	for(int y = startRow; y <= endRow; y++) {
		uint64_t* currentLine = (uint64_t*)outputBuffer;
		uint64_t* nextLine = y == lastRow ? currentLine : (uint64_t*)(outputBuffer + rowPixelGap);
		uint64_t* buffer = (uint64_t*)(outputBuffer + rowPixelGap / 2);

		RecursiveBlend(4 / _resDivider, buffer, currentLine, nextLine, pixelsPerCycle, verticalBlend);

		outputBuffer += rowPixelGap;
	}
}

/**
* NTSC_DecodeLine(Width, Signal, Target, Phase0)
*
* Convert NES NTSC graphics signal into RGB using integer arithmetics only.
*
* Width: Number of NTSC signal samples.
*        For a 256 pixels wide screen, this would be 256*8. 283*8 if you include borders.
*
* Signal: An array of Width samples.
*         Sample values are scaled such that black = 0 and white = 100.
*         The values are calculated from the set of terminated voltage measurements by lidnariq
*         with the following formula:
*                   floor(((voltage - signal_blank) / (signal_white - signal_blank)) * 100)
*         Where:
*                   signal_blank = unemphasized $0D = 0.312
*                   signal_white = unemphasized $20 = 1.100
*
* Target: Pointer to a storage for Width RGB32 samples (00rrggbb).
*         Note that the function will produce a RGB32 value for _every_ half-clock-cycle.
*         This means 2264 RGB samples if you render 283 pixels per scanline (incl. borders).
*         The caller can pick and choose those columns they want from the signal
*         to render the picture at their desired resolution.
*
* Phase0: An integer in range 0-11 that describes the phase offset into colors on this scanline.
*         Would be generated from the PPU clock cycle counter at the start of the scanline.
*         In essence it conveys in one integer the same information that real NTSC signal
*         would convey in the colorburst period in the beginning of each scanline.
*/
void BisqwitNtscFilter::NtscDecodeLine(int width, const int8_t* signal, uint32_t* target, int phase0)
{
	auto Read = [=](int pos) -> char { return pos >= 0 && pos < width ? signal[pos] : 0; };
	auto Cos = [=](int pos) -> char { return _sinetable[(pos + 36) % 12 + phase0]; };
	auto Sin = [=](int pos) -> char { return _sinetable[(pos + 36) % 12 + 3 + phase0]; };

	int ysum = _brightness, isum = 0, qsum = 0;
	int leftOverscan = GetOverscan().Left * 8;
	int rightOverscan = width - GetOverscan().Right * 8;

	int maxFilter = std::max(_yWidth, std::max(_iWidth, _qWidth)) / 2;

	for(int s = -maxFilter; s < rightOverscan; s++) {
		int sy = s + _yWidth / 2;
		int si = s + _iWidth / 2;
		int sq = s + _qWidth / 2;
		ysum += Read(sy) - Read(sy - _yWidth);
		isum += Read(si) * Cos(si) - Read(si - _iWidth) * Cos(si - _iWidth);
		qsum += Read(sq) * Sin(sq) - Read(sq - _qWidth) * Sin(sq - _qWidth);

		if(s >= leftOverscan && !(s % _resDivider)) {
			int r = std::min(255, std::max(0, (ysum*_y + isum*_ir + qsum*_qr) / 65536));
			int g = std::min(255, std::max(0, (ysum*_y + isum*_ig + qsum*_qg) / 65536));
			int b = std::min(255, std::max(0, (ysum*_y + isum*_ib + qsum*_qb) / 65536));

			*target = 0xFF000000 | (r << 16) | (g << 8) | b;
			target++;
		}
	}
}
