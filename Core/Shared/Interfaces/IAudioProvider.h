#pragma once
#include "pch.h"

class IAudioProvider
{
public:
	virtual void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) = 0;
};