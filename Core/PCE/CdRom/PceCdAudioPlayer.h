#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Utilities/Audio/HermiteResampler.h"
#include "Utilities/ISerializable.h"

class Emulator;
class PceCdRom;
struct DiscInfo;

class PceCdAudioPlayer final : public IAudioProvider, public ISerializable
{
	Emulator* _emu = nullptr;
	DiscInfo* _disc = nullptr;
	PceCdRom* _cdrom = nullptr;

	PceCdAudioPlayerState _state = {};

	vector<int16_t> _samplesToPlay;
	uint32_t _clockCounter = 0;
	uint32_t _seekDelay = 0;
	
	HermiteResampler _resampler;
	
	void PlaySample();
	void ProcessAudioPlaybackStart();

public:
	PceCdAudioPlayer(Emulator* emu, PceCdRom* cdrom, DiscInfo& disc);

	void Play(uint32_t startSector, bool pause);
	void SetEndPosition(uint32_t endSector, CdPlayEndBehavior endBehavior);
	void Stop() { _state.Status = CdAudioStatus::Stopped; }
	void Pause() { _state.Status = CdAudioStatus::Paused; }
	void SetIdle() { _state.Status = CdAudioStatus::Inactive; }
	
	PceCdAudioPlayerState& GetState() { return _state; }

	CdAudioStatus GetStatus() { return _state.Status; }
	uint32_t GetCurrentSector() { return _state.CurrentSector; }

	__forceinline void Exec()
	{
		_clockCounter += 3;
		if(_clockCounter > 487) {
			_clockCounter -= 487;
			if(_seekDelay == 0) {
				//Output one sample every 487 master clocks (~44101.1hz)
				PlaySample();
			} else {
				if(_seekDelay <= 487) {
					_seekDelay = 0;
					ProcessAudioPlaybackStart();
				} else {
					_seekDelay -= 487;
				}
			}
		}
	}

	int16_t GetLeftSample() { return _state.LeftSample; }
	int16_t GetRightSample() { return _state.RightSample; }

	void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) override;

	void Serialize(Serializer& s) override;
};
