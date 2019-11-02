#include "stdafx.h"
#include "PcmReader.h"
#include "../Utilities/VirtualFile.h"

PcmReader::PcmReader()
{
	_done = true;
	_loop = false;
	_prevLeft = 0;
	_prevRight = 0;
	_loopOffset = 8;
	_sampleRate = 48000;
	_blipLeft = blip_new(10000);
	_blipRight = blip_new(10000);
	_outputBuffer = new int16_t[20000];
}

PcmReader::~PcmReader()
{
	blip_delete(_blipLeft);
	blip_delete(_blipRight);
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

		blip_clear(_blipLeft);
		blip_clear(_blipRight);

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

		blip_clear(_blipLeft);
		blip_clear(_blipRight);
		blip_set_rates(_blipLeft, PcmReader::PcmSampleRate, _sampleRate);
		blip_set_rates(_blipRight, PcmReader::PcmSampleRate, _sampleRate);
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

		blip_add_delta(_blipLeft, samplesRead, left - _prevLeft);
		blip_add_delta(_blipRight, samplesRead, right - _prevRight);

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

	blip_end_frame(_blipLeft, samplesRead);
	blip_end_frame(_blipRight, samplesRead);
}

void PcmReader::ApplySamples(int16_t *buffer, size_t sampleCount, uint8_t volume)
{
	if(_done) {
		return;
	}

	LoadSamples((uint32_t)sampleCount * PcmReader::PcmSampleRate / _sampleRate + 1 - blip_samples_avail(_blipLeft));

	int samplesRead = blip_read_samples(_blipLeft, _outputBuffer, (int)sampleCount, 1);
	blip_read_samples(_blipRight, _outputBuffer + 1, (int)sampleCount, 1);

	for(size_t i = 0, len = samplesRead * 2; i < len; i++) {
		buffer[i] += (int16_t)((int32_t)_outputBuffer[i] * volume / 255);
	}
}

uint32_t PcmReader::GetOffset()
{
	return _fileOffset;
}