#pragma once
#include "stdafx.h"

class IVideoRecorder
{
public:
	virtual bool StartRecording(string filename, uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps) = 0;
	virtual void StopRecording() = 0;

	virtual void AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps) = 0;
	virtual void AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate) = 0;

	virtual bool IsRecording() = 0;
	virtual string GetOutputFile() = 0;
};