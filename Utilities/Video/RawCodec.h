#pragma once
#include "stdafx.h"
#include <cstring>
#include "BaseCodec.h"

class RawCodec : public BaseCodec
{
private:
	int _width = 0;
	int _height = 0;
	uint32_t _bufferSize = 0;
	uint8_t* _buffer = nullptr;

public:
	virtual bool SetupCompress(int width, int height, uint32_t compressionLevel) override
	{
		_height = height;
		_width = width;

		_bufferSize = width * height * 3;
		_buffer = new uint8_t[(_bufferSize + 1) & ~1];
		memset(_buffer, 0, (_bufferSize + 1) & ~1);

		return true;
	}

	virtual int CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData) override
	{
		*compressedData = _buffer;

		//Convert raw frame to BMP/DIB format (row order is reversed)
		uint8_t* buffer = _buffer;
		frameData += (_height - 1) * _width * 4;
		for(int y = 0; y < _height; y++) {
			for(int x = 0; x < _width; x++) {
				buffer[0] = frameData[0];
				buffer[1] = frameData[1];
				buffer[2] = frameData[2];
				frameData += 4;
				buffer += 3;
			}
			frameData -= _width * 2 * 4;
		}
		return _bufferSize;
	}

	virtual const char* GetFourCC() override
	{
		return "\0\0\0\0";
	}
};