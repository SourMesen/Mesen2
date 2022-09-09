#pragma once
#include "pch.h"
#include "Shared/Video/DrawCommand.h"

class DrawLineCommand : public DrawCommand
{
private:
	int _x, _y, _x2, _y2, _color;

protected:
	void InternalDraw()
	{
		int x = _x;
		int y = _y;
		int dx = abs(_x2 - x), sx = x < _x2 ? 1 : -1;
		int dy = abs(_y2 - y), sy = y < _y2 ? 1 : -1;
		int err = (dx > dy ? dx : -dy) / 2, e2;

		while(true) {
			DrawPixel(x, y, _color);
			if(x == _x2 && y == _y2) {
				break;
			}

			e2 = err;
			if(e2 > -dx) {
				err -= dy; x += sx;
			}
			if(e2 < dy) {
				err += dx; y += sy;
			}
		}
	}

public:
	DrawLineCommand(int x, int y, int x2, int y2, int color, int frameCount, int startFrame) :
		DrawCommand(startFrame, frameCount), _x(x), _y(y), _x2(x2), _y2(y2), _color(color)
	{
		//Invert alpha byte - 0 = opaque, 255 = transparent (this way, no need to specifiy alpha channel all the time)
		_color = (~color & 0xFF000000) | (color & 0xFFFFFF);
	}
};
