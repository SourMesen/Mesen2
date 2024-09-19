#include "pch.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/ScaleFilter.h"
#include "Utilities/xBRZ/xbrz.h"
#include "Utilities/HQX/hqx.h"
#include "Utilities/Scale2x/scalebit.h"
#include "Utilities/KreedSaiEagle/SaiEagle.h"

bool ScaleFilter::_hqxInitDone = false;

ScaleFilter::ScaleFilter(Emulator* emu, ScaleFilterType scaleFilterType, uint32_t scale)
{
	_emu = emu;
	_scaleFilterType = scaleFilterType;
	_filterScale = scale;

	if(!_hqxInitDone && _scaleFilterType == ScaleFilterType::HQX) {
		hqxInit();
		_hqxInitDone = true;
	}
}

ScaleFilter::~ScaleFilter()
{
	delete[] _outputBuffer;
}

uint32_t ScaleFilter::GetScale()
{
	return _filterScale;
}

uint32_t ScaleFilter::ApplyBrightness(uint32_t argb, uint8_t brightness)
{
	uint8_t r = ((argb & 0xFF0000) >> 16) * brightness / 255;
	uint8_t g = ((argb & 0xFF00) >> 8) * brightness / 255;
	uint8_t b = (argb & 0xFF) * brightness / 255;

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void ScaleFilter::ApplyLcdGridFilter(uint32_t* inputArgbBuffer)
{
	VideoConfig& cfg = _emu->GetSettings()->GetVideoConfig();
	uint8_t topLeft = (uint8_t)(cfg.LcdGridTopLeftBrightness * 255);
	uint8_t topRight = (uint8_t)(cfg.LcdGridTopRightBrightness * 255);
	uint8_t bottomLeft = (uint8_t)(cfg.LcdGridBottomLeftBrightness * 255);
	uint8_t bottomRight = (uint8_t)(cfg.LcdGridBottomRightBrightness * 255);

	//Rotate lcd effect as needed
	uint32_t screenRotation = _emu->GetSettings()->GetVideoConfig().ScreenRotation;
	_emu->GetScreenRotationOverride(screenRotation);

	uint8_t offset = screenRotation / 90;
	for(int i = 0; i < offset; i++) {
		uint8_t orgTopLeft = topLeft;
		topLeft = topRight;
		topRight = bottomRight;
		bottomRight = bottomLeft;
		bottomLeft = orgTopLeft;
	}

	for(uint32_t y = 0; y < _height; y++) {
		for(uint32_t x = 0; x < _width; x++) {
			uint32_t srcColor = inputArgbBuffer[y * _width + x];
			
			uint32_t pos = y * _width * _filterScale * 2 + x * _filterScale;
			_outputBuffer[pos] = ApplyBrightness(srcColor, topLeft);
			_outputBuffer[pos + 1] = ApplyBrightness(srcColor, topRight);
			_outputBuffer[pos + _width * _filterScale] = ApplyBrightness(srcColor, bottomLeft);
			_outputBuffer[pos + _width * _filterScale + 1] = ApplyBrightness(srcColor, bottomRight);
		}
	}
}

void ScaleFilter::ApplyPrescaleFilter(uint32_t *inputArgbBuffer)
{
	uint32_t* outputBuffer = _outputBuffer;

	for(uint32_t y = 0; y < _height; y++) {
		for(uint32_t x = 0; x < _width; x++) {
			for(uint32_t i = 0; i < _filterScale; i++) {
				*(outputBuffer++) = *inputArgbBuffer;
			}
			inputArgbBuffer++;
		}
		for(uint32_t i = 1; i < _filterScale; i++) {
			memcpy(outputBuffer, outputBuffer - _width*_filterScale, _width*_filterScale *4);
			outputBuffer += _width*_filterScale;
		}
	}
}

void ScaleFilter::UpdateOutputBuffer(uint32_t width, uint32_t height)
{
	if(!_outputBuffer || width != _width || height != _height) {
		delete[] _outputBuffer;
		_width = width;
		_height = height;
		_outputBuffer = new uint32_t[_width*_height*_filterScale*_filterScale];
	}
}

uint32_t* ScaleFilter::ApplyFilter(uint32_t *inputArgbBuffer, uint32_t width, uint32_t height)
{
	UpdateOutputBuffer(width, height);

	if(_scaleFilterType == ScaleFilterType::xBRZ) {
		xbrz::scale(_filterScale, inputArgbBuffer, _outputBuffer, width, height, xbrz::ColorFormat::ARGB);
	} else if(_scaleFilterType == ScaleFilterType::HQX) {
		hqx(_filterScale, inputArgbBuffer, _outputBuffer, width, height);
	} else if(_scaleFilterType == ScaleFilterType::Scale2x) {
		scale(_filterScale, _outputBuffer, width*sizeof(uint32_t)*_filterScale, inputArgbBuffer, width*sizeof(uint32_t), 4, width, height);
	} else if(_scaleFilterType == ScaleFilterType::_2xSai) {
		twoxsai_generic_xrgb8888(width, height, inputArgbBuffer, width, _outputBuffer, width * _filterScale);
	} else if(_scaleFilterType == ScaleFilterType::Super2xSai) {
		supertwoxsai_generic_xrgb8888(width, height, inputArgbBuffer, width, _outputBuffer, width * _filterScale);
	} else if(_scaleFilterType == ScaleFilterType::SuperEagle) {
		supereagle_generic_xrgb8888(width, height, inputArgbBuffer, width, _outputBuffer, width * _filterScale);
	} else if(_scaleFilterType == ScaleFilterType::Prescale) {
		ApplyPrescaleFilter(inputArgbBuffer);
	} else if(_scaleFilterType == ScaleFilterType::LcdGrid) {
		ApplyLcdGridFilter(inputArgbBuffer);
	}

	return _outputBuffer;
}

unique_ptr<ScaleFilter> ScaleFilter::GetScaleFilter(Emulator* emu, VideoFilterType filter)
{
	unique_ptr<ScaleFilter> scaleFilter;
	switch(filter) {
		default: break;

		case VideoFilterType::xBRZ2x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::xBRZ, 2)); break;
		case VideoFilterType::xBRZ3x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::xBRZ, 3)); break;
		case VideoFilterType::xBRZ4x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::xBRZ, 4)); break;
		case VideoFilterType::xBRZ5x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::xBRZ, 5)); break;
		case VideoFilterType::xBRZ6x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::xBRZ, 6)); break;
		case VideoFilterType::HQ2x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::HQX, 2)); break;
		case VideoFilterType::HQ3x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::HQX, 3)); break;
		case VideoFilterType::HQ4x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::HQX, 4)); break;
		case VideoFilterType::Scale2x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Scale2x, 2)); break;
		case VideoFilterType::Scale3x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Scale2x, 3)); break;
		case VideoFilterType::Scale4x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Scale2x, 4)); break;
		case VideoFilterType::_2xSai: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::_2xSai, 2)); break;
		case VideoFilterType::Super2xSai: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Super2xSai, 2)); break;
		case VideoFilterType::SuperEagle: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::SuperEagle, 2)); break;

		case VideoFilterType::Prescale2x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 2)); break;
		case VideoFilterType::Prescale3x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 3)); break;
		case VideoFilterType::Prescale4x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 4)); break;
		case VideoFilterType::Prescale6x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 6)); break;
		case VideoFilterType::Prescale8x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 8)); break;
		case VideoFilterType::Prescale10x: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::Prescale, 10)); break;
		
		case VideoFilterType::LcdGrid: scaleFilter.reset(new ScaleFilter(emu, ScaleFilterType::LcdGrid, 2)); break;
	}
	return scaleFilter;
}

FrameInfo ScaleFilter::GetFrameInfo(FrameInfo baseFrameInfo)
{
	FrameInfo info = baseFrameInfo;
	info.Height *= this->GetScale();
	info.Width *= this->GetScale();
	return info;
}