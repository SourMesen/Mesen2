#pragma once
#include "pch.h"
#include "BaseCodec.h"
#include "miniz.h"

class CamstudioCodec : public BaseCodec
{
private:
	uint8_t* _prevFrame = nullptr;
	uint8_t* _currentFrame = nullptr;
	uint8_t* _buffer = nullptr;

	uint32_t _compressBufferLength = 0;
	uint8_t* _compressBuffer = nullptr;
	z_stream _compressor = {};
	int _compressionLevel = 0;

	int _orgWidth = 0;
	int _rowStride = 0;
	int _height = 0;

	void LoadRow(uint8_t* inPointer, uint8_t* outPointer);

public:
	virtual ~CamstudioCodec();

	virtual bool SetupCompress(int width, int height, uint32_t compressionLevel) override;
	virtual int CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData) override;
	virtual const char* GetFourCC() override;
};