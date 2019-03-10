#include "stdafx.h"
#include "EmuSettings.h"

void EmuSettings::SetVideoConfig(VideoConfig config)
{
	_video = config;
}

VideoConfig EmuSettings::GetVideoConfig()
{
	return _video;
}

double EmuSettings::GetAspectRatio()
{
	switch(_video.AspectRatio) {
		case VideoAspectRatio::NoStretching: return 0.0;

		case VideoAspectRatio::Auto:
		{
			bool isPal = false;
			return isPal ? (9440000.0 / 6384411.0) : (128.0 / 105.0);
		}

		case VideoAspectRatio::NTSC: return 128.0 / 105.0;
		case VideoAspectRatio::PAL: return 9440000.0 / 6384411.0;
		case VideoAspectRatio::Standard: return 4.0 / 3.0;
		case VideoAspectRatio::Widescreen: return 16.0 / 9.0;
		case VideoAspectRatio::Custom: return _video.CustomAspectRatio;
	}
	return 0.0;
}