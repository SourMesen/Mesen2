#include "pch.h"
#include "AviRecorder.h"

AviRecorder::AviRecorder(VideoCodec codec, uint32_t compressionLevel)
{
	_recording = false;
	_stopFlag = false;
	_framePending = false;
	_frameBuffer = nullptr;
	_frameBufferLength = 0;
	_sampleRate = 0;
	_codec = codec;
	_compressionLevel = compressionLevel;
}

AviRecorder::~AviRecorder()
{
	if(_recording) {
		StopRecording();
	}

	if(_frameBuffer) {
		delete[] _frameBuffer;
		_frameBuffer = nullptr;
	}
}

bool AviRecorder::Init(string filename)
{
	_outputFile = filename;
	
	ofstream fileTest(filename, std::ios::out | std::ios::binary);
	if(!fileTest) {
		return false;
	}

	return true;
}

bool AviRecorder::StartRecording(uint32_t width, uint32_t height, uint32_t bpp, uint32_t audioSampleRate, double fps)
{
	if(!_recording) {
		_sampleRate = audioSampleRate;
		_width = width;
		_height = height;
		_fps = fps;
		_frameBufferLength = height * width * bpp;
		_frameBuffer = new uint8_t[_frameBufferLength];

		_aviWriter.reset(new AviWriter());
		if(!_aviWriter->StartWrite(_outputFile, _codec, width, height, bpp, (uint32_t)(_fps * 1000000), audioSampleRate, _compressionLevel)) {
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
				_framePending = false;
			}
		});

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
	}
}

bool AviRecorder::AddFrame(void* frameBuffer, uint32_t width, uint32_t height, double fps)
{
	if(_recording) {
		if(_width != width || _height != height || _fps != fps) {
			return false;
		} else {
			while(_framePending) {
				//Previous frame isn't done processing yet, wait
				std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(1));
			}

			auto lock = _lock.AcquireSafe();
			_framePending = true;
			memcpy(_frameBuffer, frameBuffer, _frameBufferLength);
			_waitFrame.Signal();
		}
	}
	return true;
}

bool AviRecorder::AddSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate)
{
	if(_recording) {
		if(_sampleRate != sampleRate) {
			return false;
		} else {
			_aviWriter->AddSound(soundBuffer, sampleCount);
		}
	}
	return true;
}

bool AviRecorder::IsRecording()
{
	return _recording;
}

string AviRecorder::GetOutputFile()
{
	return _outputFile;
}