#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"

class DrawCommand
{
private:
	int _frameCount = 0;
	int32_t _startFrame = 0;

protected:
	unordered_map<uint32_t, uint32_t>* _drawnPixels = nullptr;
	uint32_t* _argbBuffer = nullptr;
	FrameInfo _frameInfo = {};
	OverscanDimensions _overscan = {};
	bool _useIntegerScaling = false;
	bool _overwritePixels = false;
	float _xScale = 1;
	int _yScale = 1;

	virtual void InternalDraw() = 0;

	__forceinline void InternalDrawPixel(int32_t offset, int color, uint32_t alpha)
	{
		if(_drawnPixels) {
			//Log modified pixels
			if(alpha != 0xFF000000) {
				if(_drawnPixels->find(offset) == _drawnPixels->end() || _overwritePixels) {
					//When drawing on an empty background, premultiply channels & preserve alpha value
					//This is needed for hardware blending between the HUD and the game screen
					(*_drawnPixels)[offset] = 0;
					BlendColors((uint8_t*)&(*_drawnPixels)[offset], (uint8_t*)&color, true);
				} else {
					BlendColors((uint8_t*)&(*_drawnPixels)[offset], (uint8_t*)&color);
				}
			} else {
				(*_drawnPixels)[offset] = color;
			}
		} else {
			//Draw pixels directly to the buffer
			if(alpha != 0xFF000000) {
				if(_overwritePixels) {
					_argbBuffer[offset] = 0;
				}
				
				if(_argbBuffer[offset] == 0) {
					//When drawing on an empty background, premultiply channels & preserve alpha value
					//This is needed for hardware blending between the HUD and the game screen
					BlendColors((uint8_t*)&_argbBuffer[offset], (uint8_t*)&color, true);
				} else {
					BlendColors((uint8_t*)&_argbBuffer[offset], (uint8_t*)&color);
				}
			} else {
				_argbBuffer[offset] = color;
			}
		}
	}

	__forceinline bool IsOutOfBounds(int32_t x, int32_t y)
	{
		int top = (int)_overscan.Top;
		int left = (int)_overscan.Left;
		return (
			x < left ||
			y < top ||
			x - left >= (int32_t)_frameInfo.Width ||
			y - top >= (int32_t)_frameInfo.Height
		);
	}

	void DrawPixel(uint32_t x, uint32_t y, int color)
	{
		uint32_t alpha = (color & 0xFF000000);
		if(alpha > 0) {
			int top = (int)_overscan.Top;
			int left = (int)_overscan.Left;

			if(_yScale == 1 && _xScale == 1) {
				if(IsOutOfBounds(x, y)) {
					return;
				}

				int32_t offset = ((int32_t)y - top) * _frameInfo.Width + (int32_t)x - left;
				InternalDrawPixel(offset, color, alpha);
			} else {
				int xPixelCount = _useIntegerScaling ? (int)std::floor(_xScale): (int)((x + 1)*_xScale) - (int)(x*_xScale);
				x = (int)(x * (_useIntegerScaling ? (int)std::floor(_xScale) : _xScale));
				y = (int)(y * _yScale);

				for(int i = 0; i < _yScale; i++) {
					for(int j = 0; j < xPixelCount; j++) {
						int32_t offset = ((int32_t)y - top + i) * _frameInfo.Width + (int32_t)x - left + j;
						if(IsOutOfBounds(x + j, y + i)) {
							//Out of bounds, skip drawing
							continue;
						}
						InternalDrawPixel(offset, color, alpha);
					}
				}
			}
		}
	}

	__forceinline void BlendColors(uint8_t output[4], uint8_t input[4], bool keepAlpha = false)
	{
		uint8_t alpha = input[3] + 1;
		uint8_t invertedAlpha = 256 - input[3];
		output[0] = (uint8_t)((alpha * input[0] + invertedAlpha * output[0]) >> 8);
		output[1] = (uint8_t)((alpha * input[1] + invertedAlpha * output[1]) >> 8);
		output[2] = (uint8_t)((alpha * input[2] + invertedAlpha * output[2]) >> 8);
		if(keepAlpha) {
			output[3] = input[3];
		} else {
			output[3] = 0xFF;
		}
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

	void Draw(unordered_map<uint32_t, uint32_t>* drawnPixels, uint32_t* argbBuffer, FrameInfo frameInfo, OverscanDimensions &overscan, uint32_t frameNumber, HudScaleFactors &scaleFactors)
	{
		if(_startFrame < 0) {
			//When no start frame was specified, start on the next drawn frame
			_startFrame = frameNumber;
		}

		if(_startFrame <= (int32_t)frameNumber) {
			_argbBuffer = argbBuffer;
			_drawnPixels = drawnPixels;
			_frameInfo = frameInfo;
			_overscan = overscan;

			if(scaleFactors.X != 0 && scaleFactors.Y != 0) {
				_xScale = scaleFactors.X;
				_yScale = scaleFactors.Y;
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