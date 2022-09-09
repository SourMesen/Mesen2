#pragma once
#include "pch.h"
#include "Shared/Video/DrawCommand.h"

class DrawPixelCommand : public DrawCommand
{
private:
	int _x, _y, _color;

protected:
	void InternalDraw()
	{
		DrawPixel(_x, _y, _color);
	}

public:
	DrawPixelCommand(int x, int y, int color, int frameCount, int startFrame) :
		DrawCommand(startFrame, frameCount), _x(x), _y(y), _color(color)
	{
		//Invert alpha byte - 0 = opaque, 255 = transparent (this way, no need to specifiy alpha channel all the time)
		_color = (~color & 0xFF000000) | (color & 0xFFFFFF);
	}
};
