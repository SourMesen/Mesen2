#include "stdafx.h"
#include "SoundMixer.h"

SoundMixer::SoundMixer()
{
	_audioDevice = nullptr;
	_sampleRate = 32000;
}

SoundMixer::~SoundMixer()
{
}

void SoundMixer::RegisterAudioDevice(IAudioDevice *audioDevice)
{
	_audioDevice = audioDevice;
}

void SoundMixer::StopAudio(bool clearBuffer)
{
	if(_audioDevice) {
		if(clearBuffer) {
			_audioDevice->Stop();
		} else {
			_audioDevice->Pause();
		}
	}
}

void SoundMixer::PlayAudioBuffer(int16_t* samples, uint32_t sampleCount)
{
	if(_audioDevice) {
		_audioDevice->PlayBuffer(samples, sampleCount, _sampleRate, true);
		_audioDevice->ProcessEndOfFrame();
	}
}
