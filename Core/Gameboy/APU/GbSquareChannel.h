#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbApu;

class GbSquareChannel final : public ISerializable
{
private:
	const uint8_t _dutySequences[4][8] = {
		{ 0, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0, 0, 1, 1 }
	};

	GbSquareState _state = {};
	GbApu* _apu = nullptr;
	
public:
	GbSquareChannel(GbApu* apu);

	GbSquareState GetState();

	bool Enabled();
	void Disable();

	void ClockSweepUnit();	
	uint16_t GetSweepTargetFrequency();

	void ClockLengthCounter();
	void ClockEnvelope();

	uint8_t GetOutput();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};