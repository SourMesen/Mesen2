#include "pch.h"
#include "Shared/Audio/AudioPlayerHud.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/DrawStringCommand.h"

static constexpr double PI = 3.14159265358979323846;

AudioPlayerHud::AudioPlayerHud(Emulator* emu)
{
	_emu = emu;
	_mixer = emu->GetSoundMixer();
	_hud = emu->GetDebugHud();

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
	if(trackInfo.Position <= 1) {
		_changeTrackPending = false;
	}
	
	_hud->DrawRectangle(0, 0, 256, 240, 0, true, 1);

	int y = 12;
	auto drawLabel = [=, &y](string label, string content) {
		if(content.size() > 0 && content[0] != 0) {
			_hud->DrawString(10, y, label, 0xBBBBBB, 0, 1);
			_hud->DrawString(57, y, content, 0xFFFFFF, 0, 1);
			y += 10;
		}
	};

	drawLabel("Game:", trackInfo.GameTitle);
	drawLabel("Artist:", trackInfo.Artist);
	drawLabel("Comment:", trackInfo.Comment);

	string position = FormatSeconds((uint32_t)trackInfo.Position);

	string track = "Track: " + std::to_string(trackInfo.TrackNumber) + " / " + std::to_string(trackInfo.TrackCount);
	TextSize size = DrawStringCommand::MeasureString(track);
	uint32_t trackPosX = 256 - size.X - 14;
	_hud->DrawString(trackPosX, 218, track, 0xFFFFFF, 0, 1);

	string trackName;
	if(trackInfo.SongTitle.size() > 0 && trackInfo.SongTitle[0] != 0) {
		trackName = trackInfo.SongTitle;
	} else {
		trackName = _emu->GetRomInfo().RomFile.GetFileName();
	}
	_hud->DrawString(15, 208, trackName, 0xFFFFFF, 0, 1, -1, trackPosX - 20);

	if(trackInfo.Length <= 0) {
		_hud->DrawString(215, 208, " " + position + "   ", 0xFFFFFF, 0, 1);
	} else {
		position += " / " + FormatSeconds((uint32_t)trackInfo.Length);
		_hud->DrawString(177, 208, " " + position + "   ", 0xFFFFFF, 0, 1);

		constexpr int barWidth = 222;
		_hud->DrawRectangle(15, 199, barWidth + 4, 6, 0xBBBBBB, false, 1);
		//_hud->DrawRectangle(16, 200, barWidth + 2, 4, 0, true, 1);
		_hud->DrawRectangle(17, 201, (int)(std::min(1.0, trackInfo.Position / trackInfo.Length) * barWidth), 2, 0x77BBFF, false, 1);
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

	int top = 191 - maxVal;
	int bottom = 190;
	int fgColor = 0x555555;
	int fgColor2 = 0x666666;
	int bgColor = 0x222222;
	_hud->DrawLine(0, top - 1, 255, top - 1, fgColor, 1);
	_hud->DrawLine(0, bottom + 1, 255, bottom + 1, fgColor, 1);
	_hud->DrawRectangle(0, top, 256, 140, bgColor, true, 1);
	_hud->DrawLine(192, top, 192, bottom, fgColor, 1);
	_hud->DrawLine(128, top, 128, bottom, fgColor, 1);
	_hud->DrawLine(64, top, 64, bottom, fgColor, 1);
	
	_hud->DrawLine(224, top, 224, bottom, fgColor2, 1);
	_hud->DrawLine(160, top, 160, bottom, fgColor2, 1);
	_hud->DrawLine(96, top, 96, bottom, fgColor2, 1);
	_hud->DrawLine(32, top, 32, bottom, fgColor2, 1);

	_hud->DrawString(3, top + 10, "20Hz", fgColor, bgColor, 1);
	_hud->DrawString(19, top + 30, "150Hz", fgColor2, bgColor, 1);
	_hud->DrawString(51, top + 10, "400Hz", fgColor, bgColor, 1);
	_hud->DrawString(83, top + 30, "700Hz", fgColor2, bgColor, 1);
	_hud->DrawString(117, top + 10, "1kHz", fgColor, bgColor, 1);
	_hud->DrawString(150, top + 30, "2kHz", fgColor2, bgColor, 1);
	_hud->DrawString(182, top + 10, "4kHz", fgColor, bgColor, 1);
	_hud->DrawString(214, top + 30, "6kHz", fgColor2, bgColor, 1);
	_hud->DrawString(228, top + 10, "20kHz", fgColor, bgColor, 1);

	if(_amplitudes.size() >= N / 2) {
		bool silent = true;
		for(int i = 0; i < 8; i++) {
			for(int j = 0; j < 32; j++) {
				double freqRange = ranges[i][1] - ranges[i][0];
				double startFreq = ranges[i][0] + freqRange * j / 32;
				double endFreq = ranges[i][0] + freqRange * (j + 1) / 32;

				int startIndex = (int)(startFreq / (_sampleRate / N));
				int endIndex = (int)(endFreq / (_sampleRate / N));

				double avgAmp = 0;
				for(int ampIndex = startIndex; ampIndex <= endIndex && ampIndex < _amplitudes.size(); ampIndex++) {
					avgAmp += _amplitudes[ampIndex];
				}
				avgAmp /= (endIndex - startIndex + 1);
				avgAmp *= ranges[i][2];
				avgAmp = std::min<double>(maxVal, avgAmp);

				if(avgAmp >= 1) {
					silent = false;
				}

				int red = std::min(255, (int)(256 * (avgAmp / maxVal) * 2));
				int green = std::max(0, std::min(255, (int)(256 * ((maxVal - avgAmp) / maxVal) * 2)));
				_hud->DrawRectangle(i*32+j, 190, 1, (int)-avgAmp, red << 16 | green << 8, true, 1);
			}
		}

		if(!silent) {
			_silenceTimer.Reset();
		} else {
			AudioConfig audioCfg = _emu->GetSettings()->GetAudioConfig();
			if(audioCfg.AudioPlayerAutoDetectSilence && _silenceTimer.GetElapsedMS() >= audioCfg.AudioPlayerSilenceDelay * 1000) {
				//Silence detected, move to next track
				_silenceTimer.Reset();
				MoveToNextTrack();
			}
		}
	}
}

void AudioPlayerHud::MoveToNextTrack()
{
	if(!_changeTrackPending) {
		_changeTrackPending = true;
		AudioPlayerActionParams params = {};

		AudioPlayerConfig cfg = _emu->GetSettings()->GetAudioPlayerConfig();
		AudioTrackInfo track = _emu->GetAudioTrackInfo();
		if(!cfg.Repeat) {
			if(cfg.Shuffle) {
				std::random_device rd;
				std::mt19937 mt(rd());
				std::uniform_int_distribution<> dist(0, track.TrackCount - 1);
				params.Action = AudioPlayerAction::SelectTrack;
				params.TrackNumber = dist(mt);
			} else {
				params.Action = AudioPlayerAction::NextTrack;
			}
		} else {
			params.Action = AudioPlayerAction::SelectTrack;
			params.TrackNumber = track.TrackNumber - 1;
		}
		_emu->ProcessAudioPlayerAction(params);
	}
}

uint32_t AudioPlayerHud::GetVolume()
{
	AudioTrackInfo info = _emu->GetAudioTrackInfo();

	if(info.Length > 0) {
		if(info.Position >= info.Length) {
			//Switch to next track
			MoveToNextTrack();
			return 0;
		} else if(info.Position >= info.Length - info.FadeLength) {
			double fadeStart = info.Length - info.FadeLength;
			double ratio = 1.0 - ((info.Position - fadeStart) / info.FadeLength);
			return (uint32_t)(ratio * _emu->GetSettings()->GetAudioPlayerConfig().Volume);
		}
	}
	return _emu->GetSettings()->GetAudioPlayerConfig().Volume;
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
