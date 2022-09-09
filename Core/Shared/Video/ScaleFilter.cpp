#include "pch.h"
#include "ScaleFilter.h"
#include "Utilities/xBRZ/xbrz.h"
#include "Utilities/HQX/hqx.h"
#include "Utilities/Scale2x/scalebit.h"
#include "Utilities/KreedSaiEagle/SaiEagle.h"

bool ScaleFilter::_hqxInitDone = false;

ScaleFilter::ScaleFilter(ScaleFilterType scaleFilterType, uint32_t scale)
{
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
	}

	return _outputBuffer;
}

unique_ptr<ScaleFilter> ScaleFilter::GetScaleFilter(VideoFilterType filter)
{
	unique_ptr<ScaleFilter> scaleFilter;
	switch(filter) {
		default: break;

		case VideoFilterType::xBRZ2x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::xBRZ, 2)); break;
		case VideoFilterType::xBRZ3x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::xBRZ, 3)); break;
		case VideoFilterType::xBRZ4x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::xBRZ, 4)); break;
		case VideoFilterType::xBRZ5x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::xBRZ, 5)); break;
		case VideoFilterType::xBRZ6x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::xBRZ, 6)); break;
		case VideoFilterType::HQ2x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::HQX, 2)); break;
		case VideoFilterType::HQ3x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::HQX, 3)); break;
		case VideoFilterType::HQ4x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::HQX, 4)); break;
		case VideoFilterType::Scale2x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Scale2x, 2)); break;
		case VideoFilterType::Scale3x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Scale2x, 3)); break;
		case VideoFilterType::Scale4x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Scale2x, 4)); break;
		case VideoFilterType::_2xSai: scaleFilter.reset(new ScaleFilter(ScaleFilterType::_2xSai, 2)); break;
		case VideoFilterType::Super2xSai: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Super2xSai, 2)); break;
		case VideoFilterType::SuperEagle: scaleFilter.reset(new ScaleFilter(ScaleFilterType::SuperEagle, 2)); break;

		case VideoFilterType::Prescale2x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 2)); break;
		case VideoFilterType::Prescale3x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 3)); break;
		case VideoFilterType::Prescale4x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 4)); break;
		case VideoFilterType::Prescale6x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 6)); break;
		case VideoFilterType::Prescale8x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 8)); break;
		case VideoFilterType::Prescale10x: scaleFilter.reset(new ScaleFilter(ScaleFilterType::Prescale, 10)); break;
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