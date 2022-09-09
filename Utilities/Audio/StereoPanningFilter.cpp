#include "pch.h"
#include "StereoPanningFilter.h"
#include <cmath>

void StereoPanningFilter::UpdateFactors(double angle)
{
	_leftChannelFactor = _baseFactor * (std::cos(angle) - std::sin(angle));
	_rightChannelFactor = _baseFactor * (std::cos(angle) + std::sin(angle));
}

void StereoPanningFilter::ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t angle)
{
	constexpr double PI = 3.14159265358979323846;
	angle = (uint32_t)(angle / 180.0 * PI);
	UpdateFactors(angle);

	for(size_t i = 0; i < sampleCount * 2; i+=2) {
		int16_t leftSample = stereoBuffer[i];
		int16_t rightSample = stereoBuffer[i+1];
		stereoBuffer[i] = (int16_t)((_leftChannelFactor * leftSample + _leftChannelFactor * rightSample) / 2);
		stereoBuffer[i+1] = (int16_t)((_rightChannelFactor * rightSample + _rightChannelFactor * leftSample) / 2);
	}
}