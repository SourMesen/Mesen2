#pragma once
#include "stdafx.h"
#include "IAudioDevice.h"

class Console;
class Equalizer;
class SoundResampler;
class WaveRecorder;

class SoundMixer 
{
private:
	IAudioDevice *_audioDevice;
	Console *_console;
	unique_ptr<Equalizer> _equalizer;
	unique_ptr<SoundResampler> _resampler;
	shared_ptr<WaveRecorder> _waveRecorder;
	int16_t *_sampleBuffer = nullptr;

	void ProcessEqualizer(int16_t *samples, uint32_t sampleCount);

public:
	SoundMixer(Console *console);
	~SoundMixer();

	void PlayAudioBuffer(int16_t *samples, uint32_t sampleCount);
	void StopAudio(bool clearBuffer = false);

	void RegisterAudioDevice(IAudioDevice *audioDevice);
	AudioStatistics GetStatistics();
	double GetRateAdjustment();

	void StartRecording(string filepath);
	void StopRecording();
	bool IsRecording();
};
