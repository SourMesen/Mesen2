#pragma once
#include "stdafx.h"

class Console;
struct blip_t;

class SoundResampler
{
private:
	Console *_console;

	double _rateAdjustment = 1.0;
	double _previousTargetRate = 0;
	uint32_t _prevSpcSampleRate = 0;
	int32_t _underTarget = 0;

	blip_t *_blipBufLeft = nullptr;
	blip_t *_blipBufRight = nullptr;
	int16_t _lastSampleLeft = 0;
	int16_t _lastSampleRight = 0;

	double GetTargetRateAdjustment();
	void UpdateTargetSampleRate(uint32_t sampleRate);

public:
	SoundResampler(Console *console);
	~SoundResampler();

	double GetRateAdjustment();

	uint32_t Resample(int16_t *inSamples, uint32_t sampleCount, uint32_t sampleRate, int16_t *outSamples);
};