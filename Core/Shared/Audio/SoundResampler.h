#pragma once
#include "pch.h"
#include "Utilities/Audio/HermiteResampler.h"

class Emulator;

class SoundResampler
{
private:
	Emulator *_emu;

	double _rateAdjustment = 1.0;
	double _previousTargetRate = 0;
	double _prevInputRate = 0;
	int32_t _underTarget = 0;

	HermiteResampler _resampler;

	double GetTargetRateAdjustment();
	void UpdateTargetSampleRate(uint32_t sourceRate, uint32_t sampleRate);

public:
	SoundResampler(Emulator *emu);
	~SoundResampler();

	double GetRateAdjustment();
	uint32_t GetTargetRate();

	uint32_t Resample(int16_t *inSamples, uint32_t sampleCount, uint32_t sourceRate, uint32_t sampleRate, int16_t *outSamples, uint32_t maxOutCount);
};