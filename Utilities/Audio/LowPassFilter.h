#pragma once
#include "pch.h"
#include <assert.h>
#include <numeric>

class LowPassFilter
{
private:
	uint8_t _prevSampleCounter = 0;
	int16_t _prevSamplesLeft[10] = { 0,0,0,0,0,0,0,0,0,0 };
	int16_t _prevSamplesRight[10] = { 0,0,0,0,0,0,0,0,0,0 };

	void UpdateSample(int16_t *buffer, size_t index, int strength, double volume, int16_t *_prevSamples)
	{
		if(strength > 0) {
			int32_t sum = std::accumulate(_prevSamples, _prevSamples + strength, 0);
			buffer[index] = (int16_t)((sum + buffer[index]) / (strength + 1) * volume);
			_prevSamples[_prevSampleCounter] = buffer[index];
		} else {
			buffer[index] = (int16_t)(buffer[index] * volume);
		}
	}

public:
	void ApplyFilter(int16_t *buffer, size_t sampleCount, int strength, double volume = 1.0f)
	{
		assert(strength <= 10);

		for(size_t i = 0; i < sampleCount*2; i+=2) {
			UpdateSample(buffer, i, strength, volume, _prevSamplesLeft);
			UpdateSample(buffer, i+1, strength, volume, _prevSamplesRight);
			if(strength > 0) {
				_prevSampleCounter = (_prevSampleCounter + 1) % strength;
			}
		}
	}
};