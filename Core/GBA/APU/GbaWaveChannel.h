#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbaApu;
class GbaConsole;

class GbaWaveChannel final : public ISerializable
{
private:
	GbaWaveState _state = {};
	GbaApu* _apu = nullptr;
	GbaConsole* _console = nullptr;
	bool _allowRamAccess = false;
	void UpdateOutput();

public:
	GbaWaveChannel(GbaApu* apu, GbaConsole* console);

	GbaWaveState& GetState();
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