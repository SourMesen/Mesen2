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
	NTSC = 1,
	BisqwitNtscQuarterRes = 2,
	BisqwitNtscHalfRes = 3,
	BisqwitNtsc = 4,
	xBRZ2x = 5,
	xBRZ3x = 6,
	xBRZ4x = 7,
	xBRZ5x = 8,
	xBRZ6x = 9,
	HQ2x = 10,
	HQ3x = 11,
	HQ4x = 12,
	Scale2x = 13,
	Scale3x = 14,
	Scale4x = 15,
	_2xSai = 16,
	Super2xSai = 17,
	SuperEagle = 18,
	Prescale2x = 19,
	Prescale3x = 20,
	Prescale4x = 21,
	Prescale6x = 22,
	Prescale8x = 23,
	Prescale10x = 24,
	Raw = 25,
	HdPack = 999
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

struct OverscanDimensions
{
	static const int ScreenWidth = 256;
	static const int ScreenHeight = 224;

	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Top = 0;
	uint32_t Bottom = 0;

	uint32_t GetPixelCount()
	{
		return GetScreenWidth() * GetScreenHeight();
	}

	uint32_t GetScreenWidth()
	{
		return 256 - Left - Right;
	}

	uint32_t GetScreenHeight()
	{
		return 224 - Top - Bottom;
	}
};

struct PictureSettings
{
	double Brightness = 0;
	double Contrast = 0;
	double Saturation = 0;
	double Hue = 0;
	double ScanlineIntensity = 0;
};

struct NtscFilterSettings
{
	double Sharpness = 0;
	double Gamma = 0;
	double Resolution = 0;
	double Artifacts = 0;
	double Fringing = 0;
	double Bleed = 0;
	bool MergeFields = false;
	bool VerticalBlend = false;
	bool KeepVerticalResolution = false;

	double YFilterLength = 0;
	double IFilterLength = 0;
	double QFilterLength = 0;
};

struct FrameInfo
{
	uint32_t Width;
	uint32_t Height;
	uint32_t OriginalWidth;
	uint32_t OriginalHeight;
	uint32_t BitsPerPixel;
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