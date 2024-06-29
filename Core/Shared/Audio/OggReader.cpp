#include "pch.h"
#include "Shared/Audio/OggReader.h"
#include "Utilities/Audio/stb_vorbis.h"
#include "Utilities/StringUtilities.h"

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

bool OggReader::Init(string filename, bool loop, uint32_t sampleRate, uint32_t startOffset, uint32_t loopPosition)
{
	_loaded = false;
	int error;
	VirtualFile file = filename;
	_fileData = vector<uint8_t>(100000);
	if(file.ReadFile(_fileData)) {
		_vorbis = stb_vorbis_open_memory(_fileData.data(), (int)_fileData.size(), &error, nullptr);
		if(_vorbis) {
			auto comments = stb_vorbis_get_comment(_vorbis);
			for (int i = 0; i < comments.comment_list_length; i++) {
				auto comment = comments.comment_list[i];
				vector<string> commentParts = StringUtilities::Split(comment, '=');
				if (commentParts.size() != 2) { continue; }
				if (commentParts[0] != "LOOPSTART") { continue; }
				uint32_t loopPositionFromFile;
				try {
					loopPositionFromFile = std::stoi(commentParts[1]);
				} catch (std::invalid_argument const& _e) {
					throw std::runtime_error("Mis-formatted loop start in .ogg");
				} catch (std::out_of_range const& _e) {
					throw std::runtime_error("Out-of-range integer loop start in .ogg");
				}
				loopPosition = loopPositionFromFile;
			}
			_loop = loop;
			if(loopPosition > 0) {
				unsigned int sampleCount = stb_vorbis_stream_length_in_samples(_vorbis);
				_loopPosition = loopPosition < sampleCount ? loopPosition : 0;
			} else {
				_loopPosition = 0;
			}
			_oggSampleRate = stb_vorbis_get_info(_vorbis).sample_rate;
			if(startOffset > 0) {
				stb_vorbis_seek(_vorbis, startOffset);
			}
			_loaded = true;
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
	if (!_loaded) {
		return;
	}
	int32_t samplesNeeded = (int32_t)sampleCount - _resampler.GetPendingCount();
	uint32_t samplesRead = 0;
	if(samplesNeeded > 0) {
		uint32_t samplesToLoad = samplesNeeded * _oggSampleRate / _sampleRate + 2;
		uint32_t samplesLoaded = (uint32_t)stb_vorbis_get_samples_short_interleaved(_vorbis, 2, _oggBuffer, samplesToLoad * 2);
		if(samplesLoaded < samplesToLoad) {
			if(_loop) {
				stb_vorbis_seek(_vorbis, _loopPosition);
				samplesLoaded += stb_vorbis_get_samples_short_interleaved(_vorbis, 2, _oggBuffer + samplesLoaded * 2, (samplesToLoad - samplesLoaded) * 2);
			} else {
				_done = true;
			}
		}
		_resampler.SetSampleRates(_oggSampleRate, _sampleRate);
		samplesRead = _resampler.Resample<false>(_oggBuffer, samplesLoaded, _outputBuffer, sampleCount);
	}
	
	uint32_t samplesToProcess = (uint32_t)samplesRead * 2;
	for(uint32_t i = 0; i < samplesToProcess; i++) {
		buffer[i] = std::clamp<int32_t>((int32_t)(_outputBuffer[i] * volume / 255) + buffer[i], INT16_MIN, INT16_MAX);
	}
}

uint32_t OggReader::GetOffset()
{
	if (!_loaded) {
		return 0;
	}
	return stb_vorbis_get_sample_offset(_vorbis);
}
