#pragma once
#include "stdafx.h"
#include "PCE/PceTypes.h"
#include "Utilities/RandomHelper.h"

class PcePsgChannel
{
private:
	PcePsgChannelState _state = {};
	uint8_t _noiseData[0x1000];
	uint16_t _noiseAddr = 0;

	uint16_t GetPeriod();

public:
	PcePsgChannel();

	PcePsgChannelState& GetState() { return _state; }

	void Run(uint32_t clocks);
	int16_t GetOutput(bool forLeftChannel, uint8_t masterVolume);
	uint16_t GetTimer();
	void Write(uint16_t addr, uint8_t value);
};
