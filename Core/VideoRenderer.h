#pragma once
#include "stdafx.h"
#include <thread>
#include "../Utilities/AutoResetEvent.h"

class IRenderingDevice;
class Console;

//TODO
//class AviRecorder;
//enum class VideoCodec;

class VideoRenderer
{
private:
	shared_ptr<Console> _console;

	AutoResetEvent _waitForRender;
	unique_ptr<std::thread> _renderThread;
	IRenderingDevice* _renderer = nullptr;
	atomic<bool> _stopFlag;

	//TODO
	//shared_ptr<AviRecorder> _aviRecorder;

	void RenderThread();

public:
	VideoRenderer(shared_ptr<Console> console);
	~VideoRenderer();

	void StartThread();
	void StopThread();

	void UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height);
	void RegisterRenderingDevice(IRenderingDevice *renderer);
	void UnregisterRenderingDevice(IRenderingDevice *renderer);

	//TODO
	/*
	void StartRecording(string filename, VideoCodec codec, uint32_t compressionLevel);
	void AddRecordingSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate);
	void StopRecording();
	bool IsRecording();*/
};