#pragma once
#include "stdafx.h"

class IAudioProvider
{
public:
	virtual void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) = 0;
};