#pragma once
#include "pch.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/Audio/HermiteResampler.h"

struct stb_vorbis;

class OggReader
{
private:
	stb_vorbis* _vorbis = nullptr;
	int16_t* _outputBuffer = nullptr;
	int16_t* _oggBuffer = nullptr;

	HermiteResampler _resampler;

	bool _loop = false;
	bool _done = false;
	
	uint32_t _loopPosition = 0;

	int _sampleRate = 0;
	int _oggSampleRate = 0;

	vector<uint8_t> _fileData;

public:
	OggReader();
	~OggReader();

	bool Init(string filename, bool loop, uint32_t sampleRate, uint32_t startOffset = 0, uint32_t loopPosition = 0);
	bool IsPlaybackOver();
	void SetSampleRate(int sampleRate);
	void SetLoopFlag(bool loop);
	void ApplySamples(int16_t* buffer, size_t sampleCount, uint8_t volume);
	uint32_t GetOffset();
};
