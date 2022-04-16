#pragma once
#include "stdafx.h"

class HermiteResampler
{
private:
	double _prevLeft[4] = {};
	double _prevRight[4] = {};
	double _rateRatio = 1.0;
	double _fraction = 0.0;

	vector<int16_t> _pendingSamples;

	__forceinline int16_t HermiteInterpolate(double values[4], double mu);
	__forceinline void PushSample(double prevValues[4], int16_t sample);

public:
	void Reset();

	void SetSampleRates(double srcRate, double dstRate);
	uint32_t GetPendingCount();

	template<bool addMode>
	uint32_t Resample(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount);
};

template uint32_t HermiteResampler::Resample<true>(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount);
template uint32_t HermiteResampler::Resample<false>(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount);