#pragma once
#include "stdafx.h"
#include "SettingTypes.h"

class EmuSettings
{
private:
	VideoConfig _video;

public:
	void SetVideoConfig(VideoConfig config);
	VideoConfig GetVideoConfig();
	double GetAspectRatio();
};