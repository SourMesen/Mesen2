#pragma once
#include "pch.h"

class HermiteResampler
{
private:
	double _prevLeft[4] = {};
	double _prevRight[4] = {};
	int32_t _volume = 256;
	double _rateRatio = 1.0;
	double _fraction = 0.0;

	int16_t _left = 0;
	int16_t _right = 0;

	vector<int16_t> _pendingSamples;

	__forceinline int16_t HermiteInterpolate(double values[4], double mu);
	__forceinline void PushSample(double prevValues[4], int16_t sample);

	template<bool addMode>
	void WriteSample(int16_t* out, uint32_t pos, int16_t left, int16_t right);

public:
	void Reset();

	void SetVolume(double volume);
	void SetSampleRates(double srcRate, double dstRate);
	uint32_t GetPendingCount();

	template<bool addMode>
	uint32_t Resample(int16_t* in, uint32_t inSampleCount, int16_t* out, size_t maxOutSampleCount, bool fillToMax = false);
};
