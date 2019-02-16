#pragma once
#include "stdafx.h"
#include "IAudioDevice.h"

class SoundMixer 
{
private:
	IAudioDevice* _audioDevice;
	uint32_t _sampleRate;

public:
	SoundMixer();
	~SoundMixer();

	void PlayAudioBuffer(int16_t *samples, uint32_t sampleCount);
	void StopAudio(bool clearBuffer = false);

	void RegisterAudioDevice(IAudioDevice *audioDevice);
};
