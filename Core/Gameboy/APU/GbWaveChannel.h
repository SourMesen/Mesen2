#pragma once
#include "pch.h"
#include "Gameboy/GbTypes.h"
#include "Gameboy/APU/GbChannelDac.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbApu;
class Gameboy;

class GbWaveChannel final : public ISerializable
{
private:
	GbWaveState _state = {};
	GbChannelDac _dac = {};
	GbApu* _apu = nullptr;
	Gameboy* _gameboy = nullptr;
	bool _allowRamAccess = false;
	void TriggerWaveRamCorruption();
	void UpdateOutput();

public:
	GbWaveChannel(GbApu* apu, Gameboy* gameboy);

	GbWaveState& GetState();
	bool Enabled();
	void Disable();
	void ResetLengthCounter();
	
	uint8_t GetRawOutput();
	double GetOutput();

	void ClockLengthCounter();

	void Exec(uint32_t clocksToRun);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void WriteRam(uint16_t addr, uint8_t value);
	uint8_t ReadRam(uint16_t addr);

	void Serialize(Serializer& s) override;
};