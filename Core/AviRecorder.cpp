#include "stdafx.h"
#include "AviRecorder.h"
#include "MessageManager.h"
#include "Console.h"
#include "EmuSettings.h"

AviRecorder::AviRecorder(shared_ptr<Console> console)
{
	_console = console;
	_recording = false;
	_stopFlag = false;
	_frameBuffer = nullptr;
	_frameBufferLength = 0;
	_sampleRate = 0;
}

AviRecorder::~AviRecorder()
{
	if(_recording) {
		StopRecording();
	}

	delete[] _frameBuffer;
}

uint32_t AviRecorder::GetFps()
{
	if(_console->GetRegion() == ConsoleRegion::Ntsc) {
		return _console->GetSettings()->GetVideoConfig().IntegerFpsMode ? 60000000 : 60098812;
	} else {
		return _console->GetSettings()->GetVideoConfig().IntegerFpsMode ? 50000000 : 50006978;
	}
}

bool AviRecorder::StartRecording(string filename, VideoCodec codec, uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, uint32_t compressionLevel)
{
	if(!_recording) {
		_outputFile = filename;
		_sampleRate = audioSampleRate;
		_width = width;
		_height = height;
		_fps = GetFps();
		_frameBufferLength = height * width * bpp;
		_frameBuffer = new uint8_t[_frameBufferLength];

		_aviWriter.reset(new AviWriter());
		if(!_aviWriter->StartWrite(filename, codec, width, height, bpp, _fps, audioSampleRate, compressionLevel)) {
			_aviWriter.reset();
			return false;
		}

		_aviWriterThread = std::thread([=]() {
			while(!_stopFlag) {
				_waitFrame.Wait();
				if(_stopFlag) {
					break;
				}

				auto lock = _lock.AcquireSafe();
				_aviWriter->AddFrame(_frameBuffer);
			}
		});

		MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStarted", _outputFile);
		_recording = true;
	}
	return true;
}

void AviRecorder::StopRecording()
{
	if(_recording) {
		_recording = false;

		_stopFlag = true;
		_waitFrame.Signal();
		_aviWriterThread.join();

		_aviWriter->EndWrite();
		_aviWriter.reset();

		MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStopped", _outputFile);
	}
}

void AviRecorder::AddFrame(void* frameBuffer, uint32_t width, uint32_t height)
{
	if(_recording) {
		if(_width != width || _height != height || _fps != GetFps()) {
			StopRecording();
		} else {
			auto lock = _lock.AcquireSafe();
			memcpy(_frameBuffer, frameBuffer, _frameBufferLength);
			_waitFrame.Signal();
		}
	}
}

void AviRecorder::AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate)
{
	if(_recording) {
		if(_sampleRate != sampleRate) {
			auto lock = _lock.AcquireSafe();
			StopRecording();
		} else {
			_aviWriter->AddSound(soundBuffer, sampleCount);
		}
	}
}

bool AviRecorder::IsRecording()
{
	return _recording;
}