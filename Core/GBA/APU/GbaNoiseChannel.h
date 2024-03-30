#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"

class GbaApu;

class GbaNoiseChannel final : public ISerializable
{
private:
	GbaNoiseState _state = {};
	GbaApu* _apu = nullptr;

public:
	GbaNoiseChannel(GbaApu* apu);
	GbaNoiseState& GetState();

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