#include "pch.h"
#include "Shared/Audio/PcmReader.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Audio/HermiteResampler.h"

PcmReader::PcmReader()
{
	_done = true;
	_loopOffset = 8;
	_outputBuffer = new int16_t[20000];
}

PcmReader::~PcmReader()
{
	delete[] _outputBuffer;
}

bool PcmReader::Init(string filename, bool loop, uint32_t startOffset)
{
	if(_file) {
		_file.close();
	}

	_file.open(filename, ios::binary);
	if(_file) {
		_file.seekg(0, ios::end);
		_fileSize = (uint32_t)_file.tellg();
		if(_fileSize < 12) {
			return false;
		}

		_file.seekg(4, ios::beg);
		uint32_t loopOffset = (uint8_t)_file.get();
		loopOffset |= ((uint8_t)_file.get()) << 8;
		loopOffset |= ((uint8_t)_file.get()) << 16;
		loopOffset |= ((uint8_t)_file.get()) << 24;

		_loopOffset = (uint32_t)loopOffset;

		_prevLeft = 0;
		_prevRight = 0;
		_done = false;
		_loop = loop;
		_fileOffset = startOffset;
		_file.seekg(_fileOffset, ios::beg);

		_leftoverSampleCount = 0;
		_pcmBuffer.clear();
		_resampler.Reset();

		return true;
	} else {
		_done = true;
		return false;
	}
}

bool PcmReader::IsPlaybackOver()
{
	return _done;
}

void PcmReader::SetSampleRate(uint32_t sampleRate)
{
	if(sampleRate != _sampleRate) {
		_sampleRate = sampleRate;

		_resampler.SetSampleRates(PcmReader::PcmSampleRate, _sampleRate);
	}
}

void PcmReader::SetLoopFlag(bool loop)
{
	_loop = loop;
}

void PcmReader::ReadSample(int16_t &left, int16_t &right)
{
	uint8_t val[4];
	_file.get(((char*)val)[0]);
	_file.get(((char*)val)[1]);
	_file.get(((char*)val)[2]);
	_file.get(((char*)val)[3]);

	left = val[0] | (val[1] << 8);
	right = val[2] | (val[3] << 8);
}

void PcmReader::LoadSamples(uint32_t samplesToLoad)
{
	uint32_t samplesRead = 0;

	int16_t left = 0;
	int16_t right = 0;
	for(uint32_t i = _fileOffset; i < _fileSize && samplesRead < samplesToLoad; i+=4) {
		ReadSample(left, right);

		_pcmBuffer.push_back(left);
		_pcmBuffer.push_back(right);

		_prevLeft = left;
		_prevRight = right;

		_fileOffset += 4;
		samplesRead++;

		if(samplesRead < samplesToLoad && i + 4 >= _fileSize) {
			if(_loop) {
				i = _loopOffset * 4 + 8;
				_fileOffset = i;
				_file.seekg(_fileOffset, ios::beg);
			} else {
				_done = true;
			}
		}
	}
}

void PcmReader::ApplySamples(int16_t *buffer, size_t sampleCount, uint8_t volume)
{
	if(_done) {
		return;
	}

	int32_t samplesNeeded = (int32_t)sampleCount - _resampler.GetPendingCount();
	if(samplesNeeded > 0) {
		uint32_t samplesToLoad = samplesNeeded * PcmReader::PcmSampleRate / _sampleRate + 2;
		LoadSamples(samplesToLoad);
	}

	uint32_t samplesRead = _resampler.Resample<false>(_pcmBuffer.data(), (uint32_t)_pcmBuffer.size() / 2, _outputBuffer, sampleCount);
	_pcmBuffer.clear();

	uint32_t samplesToProcess = (uint32_t)samplesRead * 2;
	for(uint32_t i = 0; i < samplesToProcess; i++) {
		buffer[i] += (int16_t)((int32_t)_outputBuffer[i] * volume / 255);
	}
}

uint32_t PcmReader::GetOffset()
{
	return _fileOffset;
}