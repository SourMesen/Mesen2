// This file is a part of Mesen
// It is a heavily modified version of the hardware.h/cpp file found in DOSBox's code.

#pragma once
#include "pch.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Video/BaseCodec.h"

enum class VideoCodec
{
	None = 0,
	ZMBV = 1,
	CSCD = 2,
	GIF = 3
};

class AviWriter
{
private:
	static constexpr int WaveBufferSize = 16 * 1024;
	static constexpr int AviHeaderSize = 500;

	std::unique_ptr<BaseCodec> _codec;
	ofstream _file;

	VideoCodec _codecType;

	int16_t _audiobuf[WaveBufferSize];
	uint32_t _audioPos = 0;
	uint32_t _audiorate = 0;
	uint32_t _audiowritten = 0;

	uint32_t _frames = 0;
	uint32_t _width = 0;
	uint32_t _height = 0;
	uint32_t _bpp = 0;
	uint32_t _written = 0;
	uint32_t _fps = 0;

	uint8_t* _frameBuffer = nullptr;

	vector<uint8_t> _aviIndex;
	
	SimpleLock _audioLock;

private:
	void host_writew(uint8_t* buffer, uint16_t value);
	void host_writed(uint8_t* buffer, uint32_t value);
	void WriteAviChunk(const char * tag, uint32_t size, void * data, uint32_t flags);

public:
	void AddFrame(uint8_t* frameData);
	void AddSound(int16_t * data, uint32_t sampleCount);

	bool StartWrite(string filename, VideoCodec codec, uint32_t width, uint32_t height, uint32_t bpp, uint32_t fps, uint32_t audioSampleRate, uint32_t compressionLevel);
	void EndWrite();
};