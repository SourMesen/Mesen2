#pragma once
#include "stdafx.h"

enum class ScaleFilterType
{
	xBRZ = 0,
	HQX = 1,
	Scale2x = 2,
	_2xSai = 3,
	Super2xSai = 4,
	SuperEagle = 5,
	Prescale = 6,
};

enum class VideoFilterType
{
	None = 0,
	NTSC,
	xBRZ2x,
	xBRZ3x,
	xBRZ4x,
	xBRZ5x,
	xBRZ6x,
	HQ2x,
	HQ3x,
	HQ4x,
	Scale2x,
	Scale3x,
	Scale4x,
	_2xSai,
	Super2xSai,
	SuperEagle,
	Prescale2x,
	Prescale3x,
	Prescale4x,
	Prescale6x,
	Prescale8x,
	Prescale10x
};

enum class VideoResizeFilter
{
	NearestNeighbor = 0,
	Bilinear = 1
};

enum class VideoAspectRatio
{
	NoStretching = 0,
	Auto = 1,
	NTSC = 2,
	PAL = 3,
	Standard = 4,
	Widescreen = 5,
	Custom = 6
};

struct VideoConfig
{
	double VideoScale = 2;
	double CustomAspectRatio = 1.0;
	VideoFilterType VideoFilter = VideoFilterType::NTSC;
	VideoAspectRatio AspectRatio = VideoAspectRatio::NoStretching;
	bool UseBilinearInterpolation = false;
	bool VerticalSync = false;
	bool IntegerFpsMode = false;

	double Brightness = 0;
	double Contrast = 0;
	double Hue = 0;
	double Saturation = 0;
	double ScanlineIntensity = 0;

	double NtscArtifacts = 0;
	double NtscBleed = 0;
	double NtscFringing = 0;
	double NtscGamma = 0;
	double NtscResolution = 0;
	double NtscSharpness = 0;
	bool NtscMergeFields = false;

	bool FullscreenForceIntegerScale = false;
	bool UseExclusiveFullscreen = false;
	int32_t ExclusiveFullscreenRefreshRate = 60;
};

struct OverscanDimensions
{
	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Top = 0;
	uint32_t Bottom = 0;
};

struct FrameInfo
{
	uint32_t Width;
	uint32_t Height;
};

struct ScreenSize
{
	int32_t Width;
	int32_t Height;
	double Scale;
};

struct KeyMapping
{
	uint32_t A = 0;
	uint32_t B = 0;
	uint32_t X = 0;
	uint32_t Y = 0;
	uint32_t L = 0;
	uint32_t R = 0;
	uint32_t Up = 0;
	uint32_t Down = 0;
	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Start = 0;
	uint32_t Select = 0;

	uint32_t TurboA = 0;
	uint32_t TurboB = 0;
	uint32_t TurboX = 0;
	uint32_t TurboY = 0;
	uint32_t TurboL = 0;
	uint32_t TurboR = 0;
	uint32_t TurboSelect = 0;
	uint32_t TurboStart = 0;

	bool HasKeySet()
	{
		if(A || B || X || Y || L || R || Up || Down || Left || Right || Start || Select || TurboA || TurboB || TurboX || TurboY || TurboL || TurboR || TurboStart || TurboSelect) {
			return true;
		}
		return false;
	}

private:
	bool HasKeyBinding(uint32_t* buttons, uint32_t count)
	{
		for(uint32_t i = 0; i < count; i++) {
			if(buttons[i] != 0) {
				return true;
			}
		}
		return false;
	}
};

struct KeyMappingSet
{
	KeyMapping Mapping1;
	KeyMapping Mapping2;
	KeyMapping Mapping3;
	KeyMapping Mapping4;
	uint32_t TurboSpeed = 0;

	vector<KeyMapping> GetKeyMappingArray()
	{
		vector<KeyMapping> keyMappings;
		if(Mapping1.HasKeySet()) {
			keyMappings.push_back(Mapping1);
		}
		if(Mapping2.HasKeySet()) {
			keyMappings.push_back(Mapping2);
		}
		if(Mapping3.HasKeySet()) {
			keyMappings.push_back(Mapping3);
		}
		if(Mapping4.HasKeySet()) {
			keyMappings.push_back(Mapping4);
		}
		return keyMappings;
	}
};

enum class ControllerType
{
	None = 0,
	SnesController = 1
};