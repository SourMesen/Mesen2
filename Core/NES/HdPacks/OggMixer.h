#pragma once
#include "pch.h"
#include "Shared/Interfaces/IAudioProvider.h"

class OggReader;

class OggMixer : public IAudioProvider
{
private:
	shared_ptr<OggReader> _bgm;
	vector<shared_ptr<OggReader>> _sfx;

	uint32_t _sampleRate = 0;
	uint8_t _bgmVolume = 0;
	uint8_t _sfxVolume = 0;
	uint8_t _options = 0;
	bool _paused = false;

public:
	OggMixer();
	virtual ~OggMixer() = default;

	void SetSampleRate(int sampleRate);
	
	void Reset(uint32_t sampleRate);
	bool Play(string filename, bool isSfx, uint32_t startOffset, uint32_t loopPosition);
	void SetPlaybackOptions(uint8_t options);
	void SetPausedFlag(bool paused);
	void StopBgm();
	void StopSfx();
	void SetBgmVolume(uint8_t volume);
	void SetSfxVolume(uint8_t volume);
	bool IsBgmPlaying();
	bool IsSfxPlaying();
	int32_t GetBgmOffset();

	void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) override;
};
