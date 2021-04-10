#pragma once
#include "stdafx.h"
#include "DrawCommand.h"

class DrawScreenBufferCommand : public DrawCommand
{
private:
	uint32_t _screenBuffer[256*240];

protected:
	void InternalDraw()
	{
		for(int y = 0; y < 240; y++) {
			for(int x = 0; x < 256; x++) {
				DrawPixel(x, y, _screenBuffer[(y << 8) + x]);
			}
		}
	}

public:
	DrawScreenBufferCommand(uint32_t* screenBuffer, int startFrame) : DrawCommand(startFrame, 1)
	{
		memcpy(_screenBuffer, screenBuffer, 256 * 240 * sizeof(uint32_t));
	}
};
