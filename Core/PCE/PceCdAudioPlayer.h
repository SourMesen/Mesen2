#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Utilities/Audio/HermiteResampler.h"

class PceCdRom;
struct DiscInfo;

enum class CdPlayEndBehavior
{
	Stop,
	Loop,
	Irq
};

class PceCdAudioPlayer : public IAudioProvider
{
	DiscInfo* _disc = nullptr;
	PceCdRom* _cdrom = nullptr;

	bool _playing = false;

	uint32_t _startSector = 0;
	uint32_t _endSector = 0;
	CdPlayEndBehavior _endBehavior = CdPlayEndBehavior::Stop;

	uint32_t _currentSector = 0;
	uint32_t _currentSample = 0;

	int16_t _leftSample = 0;
	int16_t _rightSample = 0;

	vector<int16_t> _samplesToPlay;
	uint32_t _clockCounter = 0;
	
	HermiteResampler _resampler;
	
	void PlaySample();

public:
	PceCdAudioPlayer(PceCdRom* cdrom, DiscInfo& disc);

	void Play(uint32_t startSector);
	void SetEndPosition(uint32_t endSector, CdPlayEndBehavior endBehavior);
	void Stop();
	
	bool IsPlaying() { return _playing; }
	uint32_t GetCurrentSector() { return _currentSector; }

	void Exec();

	int16_t GetLeftSample() { return _leftSample; }
	int16_t GetRightSample() { return _rightSample; }

	void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) override;
};
