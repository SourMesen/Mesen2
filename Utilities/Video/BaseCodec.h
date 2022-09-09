#pragma once
#include "pch.h"

class BaseCodec
{
public:
	virtual bool SetupCompress(int width, int height, uint32_t compressionLevel) = 0;
	virtual int CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData) = 0;
	virtual const char* GetFourCC() = 0;

	virtual ~BaseCodec() { }
};