#include "pch.h"
#include "Shared/Video/RotateFilter.h"

RotateFilter::RotateFilter(uint32_t angle)
{
	_angle = angle;
}

RotateFilter::~RotateFilter()
{
	if(_outputBuffer) {
		delete[] _outputBuffer;
	}
}

void RotateFilter::UpdateOutputBuffer(uint32_t width, uint32_t height)
{
	if(!_outputBuffer || width != _width || height != _height) {
		if(_outputBuffer) {
			delete[] _outputBuffer;
		}

		_width = width;
		_height = height;
		_outputBuffer = new uint32_t[_width * _height];
	}
}

uint32_t RotateFilter::GetAngle()
{
	return _angle;
}

uint32_t* RotateFilter::ApplyFilter(uint32_t* inputArgbBuffer, uint32_t width, uint32_t height)
{
	UpdateOutputBuffer(width, height);

	uint32_t* input = inputArgbBuffer;
	if(_angle == 90) {
		for(int i = (int)height - 1; i >= 0; i--) {
			for(uint32_t j = 0; j < width; j++) {
				_outputBuffer[j * height + i] = *input;
				input++;
			}
		}
	} else if(_angle == 180) {
		for(int i = (int)height - 1; i >= 0; i--) {
			for(int j = (int)width - 1; j >= 0; j--) {
				_outputBuffer[i * width + j] = *input;
				input++;
			}
		}
	} else if(_angle == 270) {
		for(uint32_t i = 0; i < height; i++) {
			for(int j = (int)width - 1; j >= 0; j--) {
				_outputBuffer[j * height + i] = *input;
				input++;
			}
		}
	}

	return _outputBuffer;
}

FrameInfo RotateFilter::GetFrameInfo(FrameInfo baseFrameInfo)
{
	FrameInfo info = baseFrameInfo;
	if((_angle % 180) != 0) {
		//90 or 270 rotation
		info.Height = baseFrameInfo.Width;
		info.Width = baseFrameInfo.Height;
	}
	return info;
}