#pragma once
#include "stdafx.h"
#include "Shared/SettingTypes.h"

class DrawCommand
{
private:
	int _frameCount = 0;
	uint32_t* _argbBuffer = nullptr;
	FrameInfo _frameInfo = {};
	OverscanDimensions _overscan = {};
	int32_t _startFrame = 0;

protected:
	bool _useIntegerScaling = false;
	float _xScale = 1;
	int _yScale = 1;

	virtual void InternalDraw() = 0;
	void DrawPixel(uint32_t x, uint32_t y, int color)
	{
		uint32_t alpha = (color & 0xFF000000);
		if(alpha > 0) {
			int top = (int)(_overscan.Top * _yScale);
			int left = (int)(_overscan.Left * _xScale);
			if(_yScale == 1 && _xScale == 1) {
				int32_t offset = ((int32_t)y - top) * _frameInfo.Width + (int32_t)x - left;
				if((int32_t)x - left >= (int32_t)_frameInfo.Width || (int32_t)y - top >= (int32_t)_frameInfo.Height || offset < 0) {
					//Out of bounds, skip drawing
					return;
				}

				if(alpha != 0xFF000000) {
					BlendColors((uint8_t*)&_argbBuffer[offset], (uint8_t*)&color);
				} else {
					_argbBuffer[offset] = color;
				}
			} else {
				int xPixelCount = _useIntegerScaling ? (int)std::floor(_xScale): (int)((x + 1)*_xScale) - (int)(x*_xScale);
				x = (int)(x * (_useIntegerScaling ? (int)std::floor(_xScale) : _xScale));
				y = (int)(y * _yScale);

				for(int i = 0; i < _yScale; i++) {
					for(int j = 0; j < xPixelCount; j++) {
						int32_t offset = ((int32_t)y - top + i) * _frameInfo.Width + (int32_t)x - left + j;
						if((int32_t)x - left + j >= (int32_t)_frameInfo.Width || (int32_t)y - top + i >= (int32_t)_frameInfo.Height || offset < 0) {
							//Out of bounds, skip drawing
							continue;
						}

						if(alpha != 0xFF000000) {
							BlendColors((uint8_t*)&_argbBuffer[offset], (uint8_t*)&color);
						} else {
							_argbBuffer[offset] = color;
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

	void Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions &overscan, uint32_t frameNumber, bool autoScale)
	{
		if(_startFrame < 0) {
			//When no start frame was specified, start on the next drawn frame
			_startFrame = frameNumber;
		}

		if(_startFrame <= (int32_t)frameNumber) {
			_argbBuffer = argbBuffer;
			_frameInfo = frameInfo;
			_overscan = overscan;

			if(autoScale) {
				float scale = _frameInfo.Width + _overscan.Left + _overscan.Right > 256 ? (_frameInfo.Width + _overscan.Left + _overscan.Right) / 256.0f : 1;
				_yScale = _frameInfo.Height + _overscan.Top + _overscan.Bottom > 240 ? (int)scale : 1;
				_xScale = (float)scale;
			} else {
				_yScale = 1;
				_xScale = 1;
			}

			InternalDraw();

			_frameCount--;
		}
	}

	bool Expired()
	{
		return _frameCount == 0;
	}
};

struct TextSize
{
	uint32_t X;
	uint32_t Y;
};