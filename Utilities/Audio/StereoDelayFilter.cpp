#include "pch.h"
#include <algorithm>
#include "StereoDelayFilter.h"

void StereoDelayFilter::ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t sampleRate, int32_t stereoDelay)
{
	size_t delaySampleCount = (int32_t)((double)stereoDelay / 1000 * sampleRate);
	if(delaySampleCount != _lastDelay) {
		_delayedSamplesLeft.clear();
		_delayedSamplesRight.clear();
	}
	_lastDelay = delaySampleCount;
	
	for(size_t i = 0; i < sampleCount * 2; i+=2) {
		_delayedSamplesLeft.push_back(stereoBuffer[i]);
		_delayedSamplesRight.push_back(stereoBuffer[i+1]);
	}

	if(_delayedSamplesLeft.size() > delaySampleCount) {
		size_t samplesToInsert = std::max<size_t>(_delayedSamplesLeft.size() - delaySampleCount, sampleCount);

		for(size_t i = sampleCount - samplesToInsert; i < sampleCount; i++) {
			stereoBuffer[i*2] = (stereoBuffer[i*2] + stereoBuffer[i*2+1]) / 2;
			stereoBuffer[i*2+1] = (_delayedSamplesRight.front() + _delayedSamplesLeft.front()) / 2;
			_delayedSamplesLeft.pop_front();
			_delayedSamplesRight.pop_front();
		}
	}
}
