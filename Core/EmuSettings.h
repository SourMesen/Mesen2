#pragma once
#include "stdafx.h"
#include "SettingTypes.h"

class EmuSettings
{
private:
	VideoConfig _video;
	AudioConfig _audio;
	string _audioDevice;

public:
	void SetVideoConfig(VideoConfig config);
	VideoConfig GetVideoConfig();

	void SetAudioConfig(AudioConfig config);
	AudioConfig GetAudioConfig();

	uint32_t GetEmulationSpeed();
	double GetAspectRatio();
};