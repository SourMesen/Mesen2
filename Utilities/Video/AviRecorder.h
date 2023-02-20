#pragma once
#include "pch.h"
#include <thread>
#include "Utilities/AutoResetEvent.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Video/AviWriter.h"
#include "Utilities/Video/IVideoRecorder.h"

class AviRecorder final : public IVideoRecorder
{
private:
	std::thread _aviWriterThread;
	
	unique_ptr<AviWriter> _aviWriter;

	string _outputFile;
	SimpleLock _lock;
	AutoResetEvent _waitFrame;

	atomic<bool> _stopFlag;
	atomic<bool> _framePending;

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

	bool Init(string filename) override;
	bool StartRecording(uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps) override;
	void StopRecording() override;

	bool AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps) override;
	bool AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate) override;

	bool IsRecording() override;
	string GetOutputFile() override;
};