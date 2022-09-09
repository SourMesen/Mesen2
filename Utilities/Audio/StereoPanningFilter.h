#pragma once
#include "pch.h"

class StereoPanningFilter
{
private:
	const double _baseFactor = 0.70710678118654752440084436210485; // == sqrt(2)/2
	double _leftChannelFactor = 0;
	double _rightChannelFactor = 0;

	void UpdateFactors(double angle);

public:
	void ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, uint32_t angle);
};