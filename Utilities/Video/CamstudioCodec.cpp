//This is based on the code in lsnes' cscd.cpp file
//A few modifications were done to improve compression speed
#include "pch.h"
#include <cstring>
#include "CamstudioCodec.h"
#include "miniz.h"

CamstudioCodec::~CamstudioCodec()
{
	delete[] _prevFrame;
	delete[] _currentFrame;
	delete[] _buffer;
	delete[] _compressBuffer;
	deflateEnd(&_compressor);
}

bool CamstudioCodec::SetupCompress(int width, int height, uint32_t compressionLevel)
{
	_compressionLevel = compressionLevel;
	_orgWidth = width;

	if(width % 4 != 0) {
		_rowStride = ((int)((width * 24 + 31) / 32 * 4));
	} else {
		_rowStride = width*3;
	}
	_height = height;

	_prevFrame = new uint8_t[_rowStride*_height]; //24-bit RGB
	_currentFrame = new uint8_t[_rowStride*_height]; //24-bit RGB
	_buffer = new uint8_t[_rowStride*_height]; //24-bit RGB

	_compressBufferLength = compressBound(_rowStride*_height) + 2;
	_compressBuffer = new uint8_t[_compressBufferLength];
	
	memset(_prevFrame, 0, _rowStride * _height);
	memset(_currentFrame, 0, _rowStride * _height);
	memset(_buffer, 0, _rowStride * _height);
	memset(_compressBuffer, 0, _compressBufferLength);
	
	deflateInit(&_compressor, compressionLevel);

	return true;
}

void CamstudioCodec::LoadRow(uint8_t* inPointer, uint8_t* outPointer)
{
	for(int x = 0; x < _orgWidth; x++) {
		outPointer[0] = inPointer[0];
		outPointer[1] = inPointer[1];
		outPointer[2] = inPointer[2];
		outPointer += 3;
		inPointer += 4;
	}
}

int CamstudioCodec::CompressFrame(bool isKeyFrame, uint8_t *frameData, uint8_t** compressedData)
{
	deflateReset(&_compressor);

	_compressor.next_out = _compressBuffer + 2;
	_compressor.avail_out = _compressBufferLength - 2;

	_compressBuffer[0] = (isKeyFrame ? 0x03 : 0x02) | (_compressionLevel << 4);
	_compressBuffer[1] = 8; //8-bit per color

	uint8_t* rowBuffer = _currentFrame;
	for(int y = 0; y < _height; y++) {
		LoadRow(frameData + (_height - y - 1) * _orgWidth * 4, rowBuffer);
		rowBuffer += _rowStride;
	}

	if(isKeyFrame) {
		_compressor.next_in = _currentFrame;
	} else {
		for(int i = 0, len = _rowStride * _height; i < len; i++) {
			_buffer[i] = _currentFrame[i] - _prevFrame[i];
		}
		_compressor.next_in = _buffer;
	}

	memcpy(_prevFrame, _currentFrame, _rowStride*_height);
	
	_compressor.avail_in = _height * _rowStride;
	deflate(&_compressor, MZ_FINISH);
	
	*compressedData = _compressBuffer;
	return _compressor.total_out + 2;
}

const char* CamstudioCodec::GetFourCC()
{
	return "CSCD";
}