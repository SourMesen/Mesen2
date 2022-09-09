#pragma once
#include "pch.h"
#include <deque>

class ReverbDelay
{
private:
	std::deque<int16_t> _samples;
	uint32_t _delay = 0;
	double _decay = 0;

public:
	void SetParameters(double delay, double decay, int32_t sampleRate)
	{
		uint32_t delaySampleCount = (uint32_t)(delay / 1000 * sampleRate);
		if(delaySampleCount != _delay || decay != _decay) {
			_delay = delaySampleCount;
			_decay = decay;
			_samples.clear();
		}
	}

	void Reset()
	{
		_samples.clear();
	}

	void AddSamples(int16_t* buffer, size_t sampleCount)
	{
		for(size_t i = 0; i < sampleCount; i++) {
			_samples.push_back(buffer[i*2]);
		}
	}

	void ApplyReverb(int16_t* buffer, size_t sampleCount)
	{
		if(_samples.size() > _delay) {
			size_t samplesToInsert = std::min<size_t>(_samples.size() - _delay, sampleCount);

			for(size_t j = sampleCount - samplesToInsert; j < sampleCount; j++) {
				buffer[j*2] += (int16_t)((double)_samples.front() * _decay);
				_samples.pop_front();
			}
		}
	}
};

class ReverbFilter
{
private:
	ReverbDelay _delay[10];

public:
	void ResetFilter();
	void ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t sampleRate, double reverbStrength, double reverbDelay);
};