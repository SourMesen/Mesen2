#include "pch.h"
#include "Shared/Audio/WaveRecorder.h"
#include "Shared/MessageManager.h"

WaveRecorder::WaveRecorder(string outputFile, uint32_t sampleRate, bool isStereo)
{
	_stream = ofstream(outputFile, ios::out | ios::binary);
	_outputFile = outputFile;
	_streamSize = 0;
	_sampleRate = sampleRate;
	_isStereo = isStereo;
	
	if(_stream) {
		WriteHeader();
		MessageManager::DisplayMessage("SoundRecorder", "SoundRecorderStarted", _outputFile);
	}
}

WaveRecorder::~WaveRecorder()
{
	CloseFile();
}

void WaveRecorder::WriteHeader()
{
	_stream << "RIFF";
	uint32_t size = 0;
	_stream.write((char*)&size, sizeof(size));

	_stream << "WAVE";
	_stream << "fmt ";

	uint32_t chunkSize = 16;
	_stream.write((char*)&chunkSize, sizeof(chunkSize));

	uint16_t format = 1; //PCM
	uint16_t channelCount = _isStereo ? 2 : 1;
	uint16_t bytesPerSample = 2;
	uint16_t blockAlign = channelCount * bytesPerSample;
	uint32_t byteRate = _sampleRate * channelCount * bytesPerSample;
	uint16_t bitsPerSample = bytesPerSample * 8;

	_stream.write((char*)&format, sizeof(format));
	_stream.write((char*)&channelCount, sizeof(channelCount));
	_stream.write((char*)&_sampleRate, sizeof(_sampleRate));
	_stream.write((char*)&byteRate, sizeof(byteRate));

	_stream.write((char*)&blockAlign, sizeof(blockAlign));
	_stream.write((char*)&bitsPerSample, sizeof(bitsPerSample));

	_stream << "data";
	_stream.write((char*)&size, sizeof(size));
}

bool WaveRecorder::WriteSamples(int16_t * samples, uint32_t sampleCount, uint32_t sampleRate, bool isStereo)
{
	if(_sampleRate != sampleRate || _isStereo != isStereo) {
		//Format changed, stop recording
		return false;
	} else {
		uint32_t sampleBytes = sampleCount * (isStereo ? 4 : 2);
		_stream.write((char*)samples, sampleBytes);
		_streamSize += sampleBytes;
		return true;
	}
}

void WaveRecorder::UpdateSizeValues()
{
	_stream.seekp(4, ios::beg);
	uint32_t fileSize = _streamSize + 36;
	_stream.write((char*)&fileSize, sizeof(fileSize));

	_stream.seekp(40, ios::beg);
	_stream.write((char*)&_streamSize, sizeof(_streamSize));
}

void WaveRecorder::CloseFile()
{
	if(_stream && _stream.is_open()) {
		UpdateSizeValues();
		_stream.close();

		MessageManager::DisplayMessage("SoundRecorder", "SoundRecorderStopped", _outputFile);
	}
}
