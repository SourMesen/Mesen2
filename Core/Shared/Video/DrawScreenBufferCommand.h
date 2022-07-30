#pragma once
#include "stdafx.h"
#include "DrawCommand.h"

class DrawScreenBufferCommand : public DrawCommand
{
private:
	uint32_t* _screenBuffer = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;

protected:
	void InternalDraw()
	{
		for(uint32_t y = 0; y < _height; y++) {
			for(uint32_t x = 0; x < _width; x++) {
				DrawPixel(x, y, _screenBuffer[(y * _width) + x]);
			}
		}
	}

public:
	DrawScreenBufferCommand(uint32_t width, uint32_t height, int startFrame) : DrawCommand(startFrame, 1, false, true)
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
