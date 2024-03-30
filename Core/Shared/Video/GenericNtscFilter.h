#pragma once
#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/ColorUtilities.h"
#include "Utilities/NTSC/snes_ntsc.h"
#include "Utilities/NTSC/sms_ntsc.h"

class GenericNtscFilter
{
private:
	Emulator* _emu = nullptr;

	snes_ntsc_setup_t _ntscSetup = {};
	snes_ntsc_t _ntscData = {};
	uint16_t* _inputBuffer = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;

	void UpdateBufferSize(uint32_t width, uint32_t height)
	{
		if(_width != width || _height != height) {
			if(_inputBuffer) {
				delete[] _inputBuffer;
			}

			_width = width;
			_height = height;
			_inputBuffer = new uint16_t[width * height];
		}
	}

public:
	GenericNtscFilter(Emulator* emu)
	{
		_emu = emu;

		GenericNtscFilter::InitNtscFilter(_ntscSetup, _emu->GetSettings()->GetVideoConfig());
		snes_ntsc_init(&_ntscData, &_ntscSetup);
	}

	~GenericNtscFilter()
	{
		delete[] _inputBuffer;
	}

	template<typename T>
	static bool NtscFilterOptionsChanged(T& ntscSetup, VideoConfig& cfg)
	{
		if constexpr(!std::is_same<T, sms_ntsc_setup_t>::value) {
			if((ntscSetup.merge_fields == 1) != cfg.NtscMergeFields) {
				return true;
			}
		}

		return (
			ntscSetup.hue != cfg.Hue ||
			ntscSetup.saturation != cfg.Saturation ||
			ntscSetup.brightness != cfg.Brightness ||
			ntscSetup.contrast != cfg.Contrast ||
			ntscSetup.artifacts != cfg.NtscArtifacts ||
			ntscSetup.bleed != cfg.NtscBleed ||
			ntscSetup.fringing != cfg.NtscFringing ||
			ntscSetup.gamma != cfg.NtscGamma ||
			ntscSetup.resolution != cfg.NtscResolution ||
			ntscSetup.sharpness != cfg.NtscSharpness
		);
	}

	template<typename T>
	static void InitNtscFilter(T& ntscSetup, VideoConfig& cfg)
	{
		ntscSetup.hue = cfg.Hue;
		ntscSetup.saturation = cfg.Saturation;
		ntscSetup.brightness = cfg.Brightness;
		ntscSetup.contrast = cfg.Contrast;

		ntscSetup.artifacts = cfg.NtscArtifacts;
		ntscSetup.bleed = cfg.NtscBleed;
		ntscSetup.fringing = cfg.NtscFringing;
		ntscSetup.gamma = cfg.NtscGamma;
		ntscSetup.resolution = cfg.NtscResolution;
		ntscSetup.sharpness = cfg.NtscSharpness;

		if constexpr(!std::is_same<T, sms_ntsc_setup_t>::value) {
			ntscSetup.merge_fields = (int)cfg.NtscMergeFields;
		}
	}

	void ApplyFilter(uint32_t* inOut, uint32_t inWidth, uint32_t inHeight, int phase)
	{
		if(GenericNtscFilter::NtscFilterOptionsChanged(_ntscSetup, _emu->GetSettings()->GetVideoConfig())) {
			GenericNtscFilter::InitNtscFilter(_ntscSetup, _emu->GetSettings()->GetVideoConfig());
			snes_ntsc_init(&_ntscData, &_ntscSetup);
		}

		uint32_t outWidth = SNES_NTSC_OUT_WIDTH(inWidth);
		UpdateBufferSize(inWidth, inHeight);

		//Convert RGB888 to RGB555
		for(uint32_t i = 0; i < inWidth * inHeight; i++) {
			_inputBuffer[i] = ColorUtilities::Rgb888To555(inOut[i]);
		}

		snes_ntsc_blit(&_ntscData, _inputBuffer, inWidth, phase, inWidth, inHeight, inOut, outWidth * sizeof(uint32_t));
	}
};
