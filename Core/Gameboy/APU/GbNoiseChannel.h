#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/APU/GbChannelDac.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbApu;

class GbNoiseChannel final : public ISerializable
{
private:
	GbNoiseState _state = {};
	GbChannelDac _dac = {};
	GbApu* _apu = nullptr;

public:
	GbNoiseChannel(GbApu* apu);
	GbNoiseState& GetState();

	void UpdateOutput();
	bool Enabled();
	void Disable();
	void ResetLengthCounter();

	void ClockLengthCounter();
	void ClockEnvelope();

	uint8_t GetRawOutput();
	double GetOutput();
	uint32_t GetPeriod();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};