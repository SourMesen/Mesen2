#pragma once
#include "stdafx.h"
#include "../Utilities/stb_vorbis.h"
#include "../Utilities/blip_buf.h"

class PcmReader
{
private:
	static constexpr int PcmSampleRate = 44100;
	static constexpr int SamplesToRead = 100;

	int16_t* _outputBuffer;

	ifstream _file;
	uint32_t _fileOffset;
	uint32_t _fileSize;
	uint32_t _loopOffset;

	int16_t _prevLeft;
	int16_t _prevRight;

	bool _loop;
	bool _done;

	blip_t* _blipLeft;
	blip_t* _blipRight;

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
