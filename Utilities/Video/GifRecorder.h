#pragma once
#include "pch.h"
#include "Utilities/Video/IVideoRecorder.h"

struct GifWriter;

class GifRecorder final : public IVideoRecorder
{
private:
	std::unique_ptr<GifWriter> _gif;
	bool _recording = false;
	uint32_t _frameCounter = 0;
	string _outputFile;
	uint32_t _width = 0;
	uint32_t _height = 0;
	double _fps = 0;

public:
	GifRecorder();
	virtual ~GifRecorder();

	bool Init(string filename) override;
	bool StartRecording(uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps) override;
	void StopRecording() override;
	bool AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps) override;
	bool AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate) override;
	bool IsRecording() override;
	string GetOutputFile() override;
};