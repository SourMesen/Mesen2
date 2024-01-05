#pragma once
#include "pch.h"
#include "Shared/Video/DrawCommand.h"

class DrawScreenBufferCommand : public DrawCommand
{
private:
	uint32_t* _screenBuffer = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;

protected:
	void InternalDraw()
	{
		int top = (int)_overscan.Top;
		int left = (int)_overscan.Left;
		int width = _frameInfo.Width;
		int srcOffset = top * _width + left;
		uint32_t bufferSize = _frameInfo.Width * _frameInfo.Height;

		for(uint32_t y = 0; y < _frameInfo.Height; y++) {
			if(y * _frameInfo.Width + width > bufferSize) {
				break;
			}
			memcpy(_argbBuffer + y * _frameInfo.Width, _screenBuffer + srcOffset + y * _width, width * sizeof(uint32_t));
		}
	}

public:
	DrawScreenBufferCommand(uint32_t width, uint32_t height, int startFrame) : DrawCommand(startFrame, 1, false)
	{
		_width = width;
		_height = height;
		_screenBuffer = new uint32_t[width * height];
	}

	void SetPixel(int index, uint32_t color)
	{
		_screenBuffer[index] = color;
	}

	virtual ~DrawScreenBufferCommand()
	{
		delete[] _screenBuffer;
	}
};
