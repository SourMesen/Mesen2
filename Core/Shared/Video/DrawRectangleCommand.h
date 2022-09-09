#pragma once
#include "pch.h"
#include "Shared/Video/DrawCommand.h"

class DrawRectangleCommand : public DrawCommand
{
private:
	int _x, _y, _width, _height, _color;
	bool _fill;

protected:
	void InternalDraw()
	{
		if(_fill) {
			for(int j = 0; j < _height; j++) {
				for(int i = 0; i < _width; i++) {
					DrawPixel(_x + i, _y + j, _color);
				}
			}
		} else {
			for(int i = 0; i < _width; i++) {
				DrawPixel(_x + i, _y, _color);
				DrawPixel(_x + i, _y + _height - 1, _color);
			}
			for(int i = 1; i < _height - 1; i++) {
				DrawPixel(_x, _y + i, _color);
				DrawPixel(_x + _width - 1, _y + i, _color);
			}
		}
	}

public:
	DrawRectangleCommand(int x, int y, int width, int height, int color, bool fill, int frameCount, int startFrame) :
		DrawCommand(startFrame, frameCount), _x(x), _y(y), _width(width), _height(height), _color(color), _fill(fill)
	{
		if(width < 0) {
			_x += width + 1;
			_width = -width;
		}
		if(height < 0) {
			_y += height + 1;
			_height = -height;
		}

		//Invert alpha byte - 0 = opaque, 255 = transparent (this way, no need to specifiy alpha channel all the time)
		_color = (~color & 0xFF000000) | (color & 0xFFFFFF);
	}
};