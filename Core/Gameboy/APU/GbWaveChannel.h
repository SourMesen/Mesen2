#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbApu;

class GbWaveChannel final : public ISerializable
{
private:
	GbWaveState _state = {};
	GbApu* _apu = nullptr;

public:
	GbWaveChannel(GbApu* apu);

	GbWaveState GetState();
	bool Enabled();
	void Disable();
	uint8_t GetOutput();

	void ClockLengthCounter();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void WriteRam(uint16_t addr, uint8_t value);
	uint8_t ReadRam(uint16_t addr);

	void Serialize(Serializer& s) override;
};