#pragma once
#include "pch.h"
#include "Core/Shared/Interfaces/IAudioDevice.h"
#include "Utilities/safe_ptr.h"
#include "Utilities/Audio/HermiteResampler.h"

class Emulator;
class Equalizer;
class SoundResampler;
class WaveRecorder;
class IAudioProvider;
class CrossFeedFilter;
class ReverbFilter;

class SoundMixer 
{
private:
	IAudioDevice *_audioDevice;
	vector<IAudioProvider*> _audioProviders;
	Emulator *_emu;
	unique_ptr<Equalizer> _equalizer;
	unique_ptr<SoundResampler> _resampler;
	safe_ptr<WaveRecorder> _waveRecorder;
	int16_t *_sampleBuffer = nullptr;

	HermiteResampler _pitchAdjust;
	int16_t* _pitchAdjustBuffer = nullptr;

	int16_t _leftSample = 0;
	int16_t _rightSample = 0;

	unique_ptr<CrossFeedFilter> _crossFeedFilter;
	unique_ptr<ReverbFilter> _reverbFilter;

	void ProcessEqualizer(int16_t *samples, uint32_t sampleCount, uint32_t targetRate);

public:
	SoundMixer(Emulator *emu);
	~SoundMixer();

	void PlayAudioBuffer(int16_t *samples, uint32_t sampleCount, uint32_t sourceRate);
	void StopAudio(bool clearBuffer = false);

	void RegisterAudioDevice(IAudioDevice *audioDevice);

	void RegisterAudioProvider(IAudioProvider* provider);
	void UnregisterAudioProvider(IAudioProvider* provider);

	AudioStatistics GetStatistics();
	double GetRateAdjustment();

	void StartRecording(string filepath);
	void StopRecording();
	bool IsRecording();
	void GetLastSamples(int16_t &left, int16_t &right);
};
