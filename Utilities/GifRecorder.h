#pragma once
#include "stdafx.h"
#include "../Utilities/IVideoRecorder.h"

struct GifWriter;

class GifRecorder : public IVideoRecorder
{
private:
	std::unique_ptr<GifWriter> _gif;
	bool _recording = false;
	uint32_t _frameCounter = 0;
	string _outputFile;

public:
	GifRecorder();
	~GifRecorder();

	bool StartRecording(string filename, uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps) override;
	void StopRecording() override;
	void AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps) override;
	void AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate) override;
	bool IsRecording() override;
	string GetOutputFile() override;
};