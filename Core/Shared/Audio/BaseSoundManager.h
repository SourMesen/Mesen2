#pragma once
#include "Core/Shared/Interfaces/IAudioDevice.h"

class BaseSoundManager : public IAudioDevice
{
public:
	void ProcessLatency(uint32_t readPosition, uint32_t writePosition);
	AudioStatistics GetStatistics();

protected:
	bool _isStereo;
	uint32_t _sampleRate = 0;

	double _averageLatency = 0;
	uint32_t _bufferSize = 0x10000;
	uint32_t _bufferUnderrunEventCount = 0;

	int32_t _cursorGaps[60];
	int32_t _cursorGapIndex = 0;
	bool _cursorGapFilled = false;

	void ResetStats();
};
