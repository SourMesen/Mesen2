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
	int32_t _startFrame;

protected:
	bool _useIntegerScaling;
	float _xScale;
	int _yScale;

	virtual void InternalDraw() = 0;
	__forceinline void DrawPixel(uint32_t x, uint32_t y, int color)
	{
		uint32_t alpha = (color & 0xFF000000);
		if(alpha > 0) {
			if(_yScale == 1) {
				int32_t offset = ((int32_t)y - _overscan.Top) * _frameInfo.Width + ((int32_t)x - _overscan.Left);
				if(offset < 0 || offset >= (int32_t)(_frameInfo.Width * _frameInfo.Height)) {
					//Out of bounds, skip drawing
					return;
				}

				if(alpha != 0xFF000000) {
					BlendColors((uint8_t*)&_argbBuffer[offset], (uint8_t*)&color);
				} else {
					_argbBuffer[offset] = color;
				}
			} else {
				int xPixelCount = _useIntegerScaling ? _yScale : (int)((x + 1)*_xScale) - (int)(x*_xScale);
				x = (int)(x * (_useIntegerScaling ? _yScale : _xScale));
				y = (int)(y * _yScale);
				int top = (int)(_overscan.Top * _yScale);
				int left = (int)(_overscan.Left * _xScale);

				for(int i = 0; i < _yScale; i++) {
					for(int j = 0; j < xPixelCount; j++) {
						int32_t offset = ((int32_t)y - top) * _frameInfo.Width + i * _frameInfo.Width + ((int32_t)x - left) + j;
						if(offset < 0 || offset >= (int32_t)(_frameInfo.Width * _frameInfo.Height)) {
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

	void Draw(uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions &overscan, uint32_t frameNumber)
	{
		if(_startFrame < 0) {
			//When no start frame was specified, start on the next drawn frame
			_startFrame = frameNumber;
		}

		if(_startFrame <= (int32_t)frameNumber) {
			_argbBuffer = argbBuffer;
			_frameInfo = frameInfo;
			_overscan = overscan;
			_yScale = _frameInfo.Width >= 512 ? 2 : 1;
			_xScale = _frameInfo.Width >= 512 ? 2.0f : 1.0f;

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