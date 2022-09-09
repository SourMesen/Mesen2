#include "pch.h"
#include "Shared/Audio/BaseSoundManager.h"

void BaseSoundManager::ProcessLatency(uint32_t readPosition, uint32_t writePosition)
{
	//Record latency between read & write cursors once per frame
	int32_t cursorGap;
	if(writePosition < readPosition) {
		cursorGap = writePosition - readPosition + _bufferSize;
	} else {
		cursorGap = writePosition - readPosition;
	}

	_cursorGaps[_cursorGapIndex] = cursorGap;
	_cursorGapIndex = (_cursorGapIndex + 1) % 60;
	if(_cursorGapIndex == 0) {
		_cursorGapFilled = true;
	}

	if(_cursorGapFilled) {
		//Once we have 60+ frames worth of data to work with, adjust playback frequency by +/- 0.5%
		//To speed up or slow down playback in order to reach our latency goal.
		uint32_t bytesPerSample = _isStereo ? 4 : 2;

		int32_t gapSum = 0;
		for(int i = 0; i < 60; i++) {
			gapSum += _cursorGaps[i];
		}
		int32_t gapAverage = gapSum / 60;

		_averageLatency = (gapAverage / bytesPerSample) / (double)_sampleRate * 1000;
	}
}

AudioStatistics BaseSoundManager::GetStatistics()
{
	AudioStatistics stats;
	stats.AverageLatency = _averageLatency;
	stats.BufferUnderrunEventCount = _bufferUnderrunEventCount;
	stats.BufferSize = _bufferSize;
	return stats;
}

void BaseSoundManager::ResetStats()
{
	_cursorGapIndex = 0;
	_cursorGapFilled = false;
	_bufferUnderrunEventCount = 0;
	_averageLatency = 0;
}
