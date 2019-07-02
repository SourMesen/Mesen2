#pragma once
#include "../Core/IAudioDevice.h"
#include "../Core/SoundMixer.h"
#include "libretro.h"

class LibretroSoundManager : public IAudioDevice
{
private:
	retro_audio_sample_t _sendAudioSample = nullptr;
	bool _skipMode = false;
	shared_ptr<Console> _console;

public:
	LibretroSoundManager(shared_ptr<Console> console)
	{
		_console = console;
		_console->GetSoundMixer()->RegisterAudioDevice(this);
	}

	~LibretroSoundManager()
	{
		_console->GetSoundMixer()->RegisterAudioDevice(nullptr);
	}

	// Inherited via IAudioDevice
	virtual void PlayBuffer(int16_t *soundBuffer, uint32_t sampleCount, uint32_t sampleRate, bool isStereo) override
	{
		if(!_skipMode && _sendAudioSample) {
			for(uint32_t i = 0; i < sampleCount; i++) {
				_sendAudioSample(soundBuffer[i*2], soundBuffer[i*2+1]);
			}
		}
	}

	void SetSendAudioSample(retro_audio_sample_t sendAudioSample)
	{
		_sendAudioSample = sendAudioSample;
	}

	void SetSkipMode(bool skip)
	{
		_skipMode = skip;
	}

	virtual void Stop() override
	{
	}

	virtual void Pause() override
	{
	}

	virtual string GetAvailableDevices() override
	{
		return string();
	}

	virtual void SetAudioDevice(string deviceName) override
	{
	}

	virtual void ProcessEndOfFrame() override
	{
	}

	virtual AudioStatistics GetStatistics() override
	{
		return AudioStatistics();
	}
};
