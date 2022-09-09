#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbApu;

class GbNoiseChannel final : public ISerializable
{
private:
	GbNoiseState _state = {};
	GbApu* _apu = nullptr;

public:
	GbNoiseChannel(GbApu* apu);
	GbNoiseState GetState();

	bool Enabled();
	void Disable();

	void ClockLengthCounter();
	void ClockEnvelope();

	uint8_t GetOutput();
	uint32_t GetPeriod();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};