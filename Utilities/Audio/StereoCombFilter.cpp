#include "pch.h"
#include "StereoCombFilter.h"

void StereoCombFilter::ApplyFilter(int16_t * stereoBuffer, size_t sampleCount, uint32_t sampleRate, int32_t delay, uint32_t strength)
{
	size_t delaySampleCount = (int32_t)((double)delay / 1000 * sampleRate);
	if(delaySampleCount != _lastDelay) {
		_delayedSamplesLeft.clear();
		_delayedSamplesRight.clear();
		for(size_t i = 0; i < delaySampleCount; i++) {
			_delayedSamplesLeft.push_back(0);
			_delayedSamplesRight.push_back(0);
		}
	}
	_lastDelay = delaySampleCount;

	double ratio = strength == 0 ? 0 : strength / 100.0;
	for(size_t i = 0; i < sampleCount * 2; i += 2) {
		_delayedSamplesLeft.push_back(stereoBuffer[i]);
		_delayedSamplesRight.push_back(stereoBuffer[i + 1]);

		int16_t delayedSample = (_delayedSamplesRight.front() + _delayedSamplesLeft.front()) / 2;
		int16_t monoSample = (stereoBuffer[i] + stereoBuffer[i + 1]) / 2;
		stereoBuffer[i] = monoSample + (int16_t)(delayedSample * ratio);
		stereoBuffer[i + 1] = monoSample - (int16_t)(delayedSample * ratio);
		_delayedSamplesLeft.pop_front();
		_delayedSamplesRight.pop_front();
	}
}
