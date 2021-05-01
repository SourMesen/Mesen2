#include "stdafx.h"
#include "Shared/Audio/AudioPlayerHud.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Emulator.h"

static constexpr double PI = 3.14159265358979323846;

AudioPlayerHud::AudioPlayerHud(Emulator* emu)
{
	_emu = emu;
	_mixer = emu->GetSoundMixer().get();
	_hud = emu->GetDebugHud().get();

	for(int i = 0; i < N; i++) {
		_hannWindow[i] = 0.5f * (1.0f - cos(2.0f * PI * (float)(i) / (float)(N - 1.0f)));
	}
}

string AudioPlayerHud::FormatSeconds(uint32_t s)
{
	string seconds = std::to_string(s % 60);
	if(seconds.size() == 1) {
		seconds = "0" + seconds;
	}
	return std::to_string(s / 60) + ":" + seconds;
}

void AudioPlayerHud::Draw()
{
	AudioTrackInfo trackInfo = _emu->GetAudioTrackInfo();
	
	_hud->DrawRectangle(0, 0, 256, 240, 0, true, 1);

	int y = 20;
	auto drawLabel = [=, &y](string label, string content) {
		if(content.size() > 0 && content[0] != 0) {
			_hud->DrawString(20, y, label, 0xBBBBBB, 0, 1);
			_hud->DrawString(70, y, content, 0xFFFFFF, 0, 1);
			y += 10;
		}
	};

	drawLabel("Game:", trackInfo.GameTitle);
	drawLabel("Track:", trackInfo.SongTitle);
	drawLabel("Artist:", trackInfo.Artist);
	drawLabel("Comment:", trackInfo.Comment);

	string position = FormatSeconds((uint32_t)trackInfo.Position);
	if(trackInfo.Length <= 0) {
		_hud->DrawString(220, 218, position, 0xFFFFFF, 0, 1);
	} else {
		position += " / " + FormatSeconds(trackInfo.Length);
		_hud->DrawString(182, 218, position, 0xFFFFFF, 0, 1);

		_hud->DrawRectangle(15, 209, 226, 6, 0xFFFFFF, false, 1);
		_hud->DrawRectangle(16, 210, 224, 4, 0, true, 1);
		_hud->DrawRectangle(17, 211, (int)(trackInfo.Position / trackInfo.Length * 222), 2, 0x44FF88, false, 1);
	}

	//Arbitrary ranges to split the graph into (8 equally sized sections on the screen that contain a specific freq range)
	static constexpr double ranges[8][3] {
		{ 20, 150, 0.5 },
		{ 150, 400, 0.5 },
		{ 400, 700, 0.75 },
		{ 700, 1000, 0.75 },
		{ 1000, 2000, 1 },
		{ 2000, 4000, 1 },
		{ 4000, 6000, 1.25 },
		{ 6000, 20000, 1.25 }
	};

	static constexpr int maxVal = 140;

	if(_amplitudes.size() >= N / 2) {
		for(int i = 0; i < 8; i++) {
			for(int j = 0; j < 32; j++) {
				double freqRange = ranges[i][1] - ranges[i][0];
				double startFreq = ranges[i][0] + freqRange * j / 32;
				double endFreq = ranges[i][0] + freqRange * (j + 1) / 32;

				int startIndex = (int)(startFreq / (_sampleRate / N));
				int endIndex = (int)(endFreq / (_sampleRate / N));

				double avgAmp = 0;
				for(int j = startIndex; j <= endIndex && j < _amplitudes.size(); j++) {
					avgAmp += _amplitudes[j];
				}
				avgAmp /= (endIndex - startIndex + 1);
				avgAmp *= ranges[i][2];
				avgAmp = std::min<double>(maxVal, avgAmp);

				int red = std::min(255, (int)(256 * (avgAmp / maxVal) * 2));
				int green = std::max(0, std::min(255, (int)(256 * ((maxVal - avgAmp) / maxVal) * 2)));
				_hud->DrawRectangle(i*32+j, 200, 1, (int)-avgAmp, red << 16 | green << 8, true, 1);
			}
		}
	}
}

void AudioPlayerHud::ProcessSamples(int16_t* samples, size_t sampleCount, uint32_t sampleRate)
{
	_sampleRate = sampleRate;
	for(int i = 0; i < sampleCount; i++) {
		_samples.push_back((samples[i * 2] + samples[i * 2 + 1]) / 2);
		if(_samples.size() > N) {
			_samples.pop_front();
		}
	}

	if(_samples.size() >= N) {
		for(int i = 0; i < N; i++) {
			_input[i] = _samples[i] * _hannWindow[i];
		}

		_fft.transform_real(_input, _out);

		_amplitudes.clear();
		for(int i = 0; i < N / 2; i++) {
			std::complex<double> c = _out[i];
			double amp = sqrt(c.real() * c.real() + c.imag() * c.imag());
			_amplitudes.push_back(amp / N);
		}
	}
}
