#include "pch.h"
#include "Utilities/Audio/WavReader.h"

unique_ptr<WavReader> WavReader::Create(uint8_t* wavData, uint32_t length)
{
	//Basic WAV file read checks (should be using FourCC logic instead, but this will do for now)
	if(!wavData || length < 100) {
		return nullptr;
	}
	if(memcmp(wavData, "RIFF", 4) || memcmp(wavData + 8, "WAVE", 4) || memcmp(wavData + 12, "fmt ", 4)) {
		return nullptr;
	}

	uint32_t riffSize = wavData[4] | (wavData[5] << 8) | (wavData[6] << 16) | (wavData[7] << 24);
	if(riffSize + 8 != length) {
		//Invalid RIFF header (length does not match file size)
		return nullptr;
	}

	uint32_t channelCount = wavData[22] | (wavData[23] << 8);
	if(channelCount != 1) {
		//Only mono files are supported at the moment
		return nullptr;
	}

	uint32_t fmtSize = wavData[16] | (wavData[17] << 8) | (wavData[18] << 16) | (wavData[19] << 24);
	if(memcmp(wavData + 20 + fmtSize, "data", 4)) {
		//Couldn't find data chunk
		return nullptr;
	}

	uint32_t dataSize = wavData[24 + fmtSize] | (wavData[25 + fmtSize] << 8) | (wavData[26 + fmtSize] << 16) | (wavData[27 + fmtSize] << 24);
	if(dataSize + 28 + fmtSize > length) {
		//data chunk is too big
		return nullptr;
	}

	uint32_t bitsPerSample = wavData[34] | (wavData[35] << 8);
	if(bitsPerSample != 16) {
		//Only support 16-bit samples for now
		return nullptr;
	}

	uint32_t sampleRate = wavData[24] | (wavData[25] << 8) | (wavData[26] << 16) | (wavData[27] << 24);

	unique_ptr<WavReader> r(new WavReader());
	r->_fileData = wavData;
	r->_fileSize = length;
	r->_fileSampleRate = sampleRate;
	r->_dataStartOffset = 28 + fmtSize;
	return r;
}

WavReader::WavReader()
{
	_done = true;
}

WavReader::~WavReader()
{
}

void WavReader::Play(uint32_t startSample)
{
	_done = false;
	_fileOffset = _dataStartOffset + startSample * 2;
}

bool WavReader::IsPlaybackOver()
{
	return _done;
}

void WavReader::ApplySamples(int16_t *buffer, size_t sampleCount, uint32_t sampleRate)
{
	if(_done) {
		return;
	}

	_resampler.SetSampleRates(_fileSampleRate, sampleRate);
	
	int32_t samplesNeeded = (int32_t)sampleCount - (int32_t)_resampler.GetPendingCount();

	vector<int16_t> stereoSamples;
	if(samplesNeeded > 0) {
		uint32_t samplesToLoad = (uint32_t)(samplesNeeded * (double)_fileSampleRate / sampleRate + 2);
		if(samplesToLoad > 0) {
			uint32_t samplesRead = 0;
			for(uint32_t i = _fileOffset; i < _fileSize && samplesRead < samplesToLoad; i += 2) {
				int16_t sample = _fileData[i] | (_fileData[i + 1] << 8);
				
				stereoSamples.push_back(sample);
				stereoSamples.push_back(sample);

				_fileOffset += 2;
				samplesRead++;

				if(samplesRead < samplesToLoad && i + 2 >= _fileSize) {
					_done = true;
					break;
				}
			}
		}
	}

	_resampler.Resample<true>(stereoSamples.data(), (uint32_t)stereoSamples.size() / 2, buffer, sampleCount);
}

int32_t WavReader::GetPosition()
{
	return _done ? -1 : (_fileOffset - _dataStartOffset) / 2;
}

uint32_t WavReader::GetSampleRate()
{
	return _fileSampleRate;
}