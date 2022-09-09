#include "pch.h"

class CrossFeedFilter
{
public:
	void ApplyFilter(int16_t* stereoBuffer, size_t sampleCount, int ratio);
};