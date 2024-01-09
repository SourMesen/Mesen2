//NTSC filter based on Bisqwit's code/algorithm
//As described here:
//http://forums.nesdev.com/viewtopic.php?p=172329
#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Utilities/AutoResetEvent.h"

class BisqwitNtscFilter : public BaseVideoFilter
{
private:
	const uint16_t _bitmaskLut[12] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800 };
	static constexpr int _signalsPerPixel = 8;
	static constexpr int _signalWidth = 258;

	std::thread _extraThread;
	AutoResetEvent _waitWork;
	atomic<bool> _stopThread;
	atomic<bool> _workDone;

	int _resDivider = 1;
	uint16_t *_ppuOutputBuffer = nullptr;
	
	/* Ywidth, Iwidth and Qwidth are the filter widths for Y,I,Q respectively.
	* All widths at 12 produce the best signal quality.
	* 12,24,24 would be the closest values matching the NTSC spec.
	* But off-spec values 12,22,26 are used here, to bring forth mild
	* "chroma dots", an artifacting common with badly tuned TVs.
	* Larger values = more horizontal blurring.
	*/
	int _yWidth, _iWidth, _qWidth;
	int _y;
	int _ir, _ig, _ib;
	int _qr, _qg, _qb;

	//To finetune hue, you would have to recalculate sinetable[]. (Coarse changes can be made with Phase0.)
	int8_t _sinetable[27]; // 8*sin(x*2pi/12)
	int8_t _signalLow[0x80];
	int8_t _signalHigh[0x80];

	uint32_t _brightness = 0;

	void RecursiveBlend(int iterationCount, uint64_t *output, uint64_t *currentLine, uint64_t *nextLine, int pixelsPerCycle, bool verticalBlend);
	
	void NtscDecodeLine(int width, const int8_t* signal, uint32_t* target, int phase0);
	
	void GenerateNtscSignal(int8_t *ntscSignal, int &phase, int rowNumber);
	void DecodeFrame(int startRow, int endRow, uint16_t *ppuOutputBuffer, uint32_t* outputBuffer, int startPhase);
	void OnBeforeApplyFilter() override;

public:
	BisqwitNtscFilter(Emulator* emu);
	virtual ~BisqwitNtscFilter();

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	HudScaleFactors GetScaleFactor() override;
};