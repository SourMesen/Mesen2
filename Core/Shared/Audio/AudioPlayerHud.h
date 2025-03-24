#pragma once
#include "pch.h"
#include <complex>
#include "Utilities/kissfft.h"
#include "Utilities/Timer.h"

class Emulator;
class SoundMixer;
class DebugHud;

class AudioPlayerHud
{
private:
	static constexpr int N = 2048*4;

	Emulator* _emu = nullptr;
	DebugHud* _hud = nullptr;
	SoundMixer* _mixer = nullptr;

	kissfft<double> _fft = kissfft<double>(N / 2, false);
	std::vector<double> _amplitudes;
	std::deque<int16_t> _samples;
	
	uint32_t _prevFrameCounter = 0;
	uint32_t _lastAudioFrame = 0;
	double _prevFps = 0;
	bool _changeTrackPending = false;

	uint32_t _sampleRate = 48000;
	double _hannWindow[N] = {};
	double _input[N] = {};
	std::complex<double> _out[N] = {};

	string FormatSeconds(uint32_t s);
	void MoveToNextTrack();

public:
	AudioPlayerHud(Emulator* emu);

	void Draw(uint32_t frameCounter, double fps);
	uint32_t GetVolume();
	void ProcessSamples(int16_t* samples, size_t sampleCount, uint32_t sampleRate);
};