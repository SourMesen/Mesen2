#pragma once
#include "stdafx.h"
#include <complex>
#include "Utilities/kissfft.h"

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
	
	uint32_t _sampleRate;
	double _hannWindow[N] = {};
	double _input[N] = {};
	std::complex<double> _out[N] = {};

	string FormatSeconds(uint32_t s);

public:
	AudioPlayerHud(Emulator* emu);

	void Draw();
	void ProcessSamples(int16_t* samples, size_t sampleCount, uint32_t sampleRate);
};