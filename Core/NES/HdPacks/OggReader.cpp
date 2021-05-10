#include "stdafx.h"
#include "NES/HdPacks/OggReader.h"
#include "Utilities/Audio/stb_vorbis.h"

OggReader::OggReader()
{
	_done = false;
	_oggBuffer = new int16_t[10000];
	_outputBuffer = new int16_t[2000];
}

OggReader::~OggReader()
{
	delete[] _oggBuffer;
	delete[] _outputBuffer;

	if(_vorbis) {
		stb_vorbis_close(_vorbis);
	}
}

bool OggReader::Init(string filename, bool loop, uint32_t sampleRate, uint32_t startOffset)
{
	int error;
	VirtualFile file = filename;
	_fileData = vector<uint8_t>(100000);
	if(file.ReadFile(_fileData)) {
		_vorbis = stb_vorbis_open_memory(_fileData.data(), (int)_fileData.size(), &error, nullptr);
		if(_vorbis) {
			_loop = loop;
			_oggSampleRate = stb_vorbis_get_info(_vorbis).sample_rate;
			if(startOffset > 0) {
				stb_vorbis_seek(_vorbis, startOffset);
			}
			return true;
		}
	}
	return false;
}

bool OggReader::IsPlaybackOver()
{
	return _done;
}

void OggReader::SetSampleRate(int sampleRate)
{
	if(sampleRate != _sampleRate) {
		_sampleRate = sampleRate;
	}
}

void OggReader::SetLoopFlag(bool loop)
{
	_loop = loop;
}

void OggReader::ApplySamples(int16_t* buffer, size_t sampleCount, uint8_t volume)
{
	int32_t samplesNeeded = (int32_t)sampleCount - _leftoverSampleCount;
	uint32_t samplesRead = 0;
	if(samplesNeeded > 0) {
		uint32_t samplesToLoad = samplesNeeded * _oggSampleRate / _sampleRate + 2;
		uint32_t samplesLoaded = (uint32_t)stb_vorbis_get_samples_short_interleaved(_vorbis, 2, _oggBuffer, samplesToLoad * 2);
		if(samplesLoaded < samplesToLoad) {
			if(_loop) {
				stb_vorbis_seek_start(_vorbis);
				samplesLoaded += stb_vorbis_get_samples_short_interleaved(_vorbis, 2, _oggBuffer + samplesLoaded * 2, samplesToLoad - samplesLoaded);
			} else {
				_done = true;
			}
		}
		_resampler.SetSampleRates(_oggSampleRate, _sampleRate);
		samplesRead = _resampler.Resample(_oggBuffer, samplesLoaded, _outputBuffer + _leftoverSampleCount * 2);
	}
	

	uint32_t samplesToProcess = std::min<uint32_t>((uint32_t)sampleCount * 2, (samplesRead + _leftoverSampleCount) * 2);
	for(uint32_t i = 0; i < samplesToProcess; i++) {
		buffer[i] += (int16_t)((int32_t)_outputBuffer[i] * volume / 255);
	}

	//Calculate count of extra samples that couldn't be mixed with the rest of the audio and copy them to the beginning of the buffer
	//These will be mixed on the next call to ApplySamples
	_leftoverSampleCount = std::max(0, (int32_t)(samplesRead + _leftoverSampleCount) - (int32_t)sampleCount);
	for(uint32_t i = 0; i < _leftoverSampleCount * 2; i++) {
		_outputBuffer[i] = _outputBuffer[samplesToProcess + i];
	}
}

uint32_t OggReader::GetOffset()
{
	return stb_vorbis_get_file_offset(_vorbis);
}