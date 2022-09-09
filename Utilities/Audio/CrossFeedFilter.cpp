#include "pch.h"
#include "CrossFeedFilter.h"

void CrossFeedFilter::ApplyFilter(int16_t *stereoBuffer, size_t sampleCount, int ratio)
{
	for(size_t i = 0; i < sampleCount; i++) {
		int16_t leftSample = stereoBuffer[0];
		int16_t rightSample = stereoBuffer[1];

		stereoBuffer[0] += rightSample * ratio / 100;
		stereoBuffer[1] += leftSample * ratio / 100;

		stereoBuffer += 2;
	}
}
