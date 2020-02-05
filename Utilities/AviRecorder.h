#pragma once
#include "stdafx.h"
#include <thread>
#include "AutoResetEvent.h"
#include "AviWriter.h"
#include "SimpleLock.h"
#include "IVideoRecorder.h"

class Console;

class AviRecorder : public IVideoRecorder
{
private:
	std::thread _aviWriterThread;
	
	unique_ptr<AviWriter> _aviWriter;

	string _outputFile;
	SimpleLock _lock;
	AutoResetEvent _waitFrame;

	atomic<bool> _stopFlag;	
	bool _recording;
	uint8_t* _frameBuffer;
	uint32_t _frameBufferLength;
	uint32_t _sampleRate;

	double _fps;
	uint32_t _width;
	uint32_t _height;

	VideoCodec _codec;
	uint32_t _compressionLevel;

public:
	AviRecorder(VideoCodec codec, uint32_t compressionLevel);
	virtual ~AviRecorder();

	bool StartRecording(string filename, uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps) override;
	void StopRecording() override;

	void AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps) override;
	void AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate) override;

	bool IsRecording() override;
	string GetOutputFile() override;
};