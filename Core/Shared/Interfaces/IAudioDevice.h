#pragma once

#include "pch.h"

struct AudioStatistics
{
	double AverageLatency = 0;
	uint32_t BufferUnderrunEventCount = 0;
	uint32_t BufferSize = 0;
};

class IAudioDevice
{
public:
	virtual ~IAudioDevice() {}
	virtual void PlayBuffer(int16_t *soundBuffer, uint32_t bufferSize, uint32_t sampleRate, bool isStereo) = 0;
	virtual void Stop() = 0;
	virtual void Pause() = 0;
	virtual void ProcessEndOfFrame() = 0;

	virtual string GetAvailableDevices() = 0;
	virtual void SetAudioDevice(string deviceName) = 0;

	virtual AudioStatistics GetStatistics() = 0;
};