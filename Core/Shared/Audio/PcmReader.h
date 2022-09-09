#pragma once
#include "pch.h"
#include "Utilities/Audio/stb_vorbis.h"
#include "Utilities/Audio/HermiteResampler.h"

class PcmReader
{
private:
	static constexpr int PcmSampleRate = 44100;
	static constexpr int SamplesToRead = 100;

	int16_t* _outputBuffer = nullptr;

	ifstream _file;
	uint32_t _fileOffset = 0;
	uint32_t _fileSize = 0;
	uint32_t _loopOffset = 0;

	int16_t _prevLeft = 0;
	int16_t _prevRight = 0;

	bool _loop = false;
	bool _done = false;

	HermiteResampler _resampler;
	vector<int16_t> _pcmBuffer;
	uint32_t _leftoverSampleCount = 0;

	uint32_t _sampleRate = 0;

	void LoadSamples(uint32_t samplesToLoad);
	void ReadSample(int16_t &left, int16_t &right);

public:
	PcmReader();
	~PcmReader();

	bool Init(string filename, bool loop, uint32_t startOffset = 0);
	bool IsPlaybackOver();
	void SetSampleRate(uint32_t sampleRate);
	void SetLoopFlag(bool loop);
	void ApplySamples(int16_t* buffer, size_t sampleCount, uint8_t volume);
	uint32_t GetOffset();
};
