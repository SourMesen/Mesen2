#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"

class GbaApu;

class GbaSquareChannel final : public ISerializable
{
private:
	static constexpr uint8_t _dutySequences[4][8] = {
		{ 0, 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 0, 0 }
	};

	GbaSquareState _state = {};
	GbaApu* _apu = nullptr;

public:
	GbaSquareChannel(GbaApu* apu);

	GbaSquareState& GetState();

	void UpdateOutput();
	bool Enabled();
	void Disable();
	void ResetLengthCounter();

	void ClockSweepUnit();	
	uint16_t GetSweepTargetFrequency();

	void ClockLengthCounter();
	void ClockEnvelope();

	uint8_t GetRawOutput();
	double GetOutput();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};