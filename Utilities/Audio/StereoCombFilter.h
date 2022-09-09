#pragma once
#include "pch.h"
#include <deque>

class StereoCombFilter
{
	std::deque<int16_t> _delayedSamplesLeft;
	std::deque<int16_t> _delayedSamplesRight;
	size_t _lastDelay = 0;

public:
	void ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t sampleRate, int32_t delay, uint32_t strength);
};