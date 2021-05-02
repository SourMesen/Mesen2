#pragma once
#include "stdafx.h"
#include "Shared/SettingTypes.h"

class DrawCommand
{
private:
	int _frameCount;
	uint32_t* _argbBuffer;
	FrameInfo _frameInfo;
	OverscanDimensions _overscan;
	uint32_t _lineWidth;
	int32_t _startFrame;

protected:
	bool _useIntegerScaling;
	float _xScale;
	int _yScale;

	virtual void InternalDraw() = 0;
	__forceinline void DrawPixel(uint32_t x, uint32_t y, int color)
	{
		if(x < _overscan.Left || x - _overscan.Left >= _frameInfo.Width || y < _overscan.Top || y - _overscan.Top >= _frameInfo.Height) {
			//Out of bounds, skip drawing
			return;
		}

		uint32_t alpha = (color & 0xFF000000);
		if(alpha > 0) {
			if(_yScale == 1) {
				if(alpha != 0xFF000000) {
					BlendColors((uint8_t*)&_argbBuffer[(y - _overscan.Top)*_lineWidth + (x - _overscan.Left)], (uint8_t*)&color);
				} else {
					_argbBuffer[(y - _overscan.Top)*_lineWidth + (x - _overscan.Left)] = color;
				}
			} else {
				int xPixelCount = _useIntegerScaling ? _yScale : (int)((x + 1)*_xScale) - (int)(x*_xScale);
				x = (int)(x * (_useIntegerScaling ? _yScale : _xScale));
				y = (int)(y * _yScale);
				int top = (int)(_overscan.Top * _yScale);
				int left = (int)(_overscan.Left * _xScale);

				for(int i = 0; i < _yScale; i++) {
					for(int j = 0; j < xPixelCount; j++) {
						if(alpha != 0xFF000000) {
							BlendColors((uint8_t*)&_argbBuffer[(y - top)*_lineWidth + i*_lineWidth + (x - left)+j], (uint8_t*)&color);
						} else {
							_argbBuffer[(y - top)*_lineWidth + i*_lineWidth + (x - left) +j] = color;
						}
					}
				}
			}
		}
	}

	__forceinline void BlendColors(uint8_t output[4], uint8_t input[4])
	{
		uint8_t alpha = input[3] + 1;
		uint8_t invertedAlpha = 256 - input[3];
		output[0] = (uint8_t)((alpha * input[0] + invertedAlpha * output[0]) >> 8);
		output[1] = (uint8_t)((alpha * input[1] + invertedAlpha * output[1]) >> 8);
		output[2] = (uint8_t)((alpha * input[2] + invertedAlpha * output[2]) >> 8);
		output[3] = 0xFF;
	}

public:
	DrawCommand(int startFrame, int frameCount, bool useIntegerScaling = false)
	{ 
		_frameCount = frameCount > 0 ? frameCount : -1;
		_startFrame = startFrame;
		_useIntegerScaling = useIntegerScaling;
	}

	virtual ~DrawCommand()
	{
	}

	void Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions &overscan, uint32_t lineWidth, uint32_t frameNumber)
	{
		if(_startFrame < 0) {
			//When no start frame was specified, start on the next drawn frame
			_startFrame = frameNumber;
		}

		if(_startFrame <= frameNumber) {
			_argbBuffer = argbBuffer;
			_frameInfo = frameInfo;
			_overscan = overscan;
			_lineWidth = lineWidth;
			_yScale = lineWidth >= 512 ? 2 : 1;
			_xScale = lineWidth >= 512 ? 2.0f : 1.0f;

			InternalDraw();

			_frameCount--;
		}
	}

	bool Expired()
	{
		return _frameCount == 0;
	}
};
