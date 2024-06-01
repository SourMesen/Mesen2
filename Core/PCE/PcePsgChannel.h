#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "Utilities/RandomHelper.h"
#include "Utilities/ISerializable.h"

class PcePsg;

class PcePsgChannel : public ISerializable
{
private:
	PcePsgChannelState _state = {};
	PcePsg* _psg = nullptr;
	uint8_t _chIndex = 0;
	uint8_t _outputOffset = 0;

	uint32_t GetNoisePeriod();
	uint32_t GetPeriod();

public:
	PcePsgChannel();

	void Init(uint8_t index, PcePsg* psg);
	void SetOutputOffset(uint8_t offset);

	PcePsgChannelState& GetState() { return _state; }

	void Run(uint32_t clocks);
	int16_t GetOutput(bool forLeftChannel, uint8_t masterVolume);
	uint16_t GetTimer();
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};
