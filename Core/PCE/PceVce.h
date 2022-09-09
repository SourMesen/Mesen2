#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "Utilities/ISerializable.h"

class PceConsole;
class Emulator;

class PceVce final : public ISerializable
{
private:
	PceVceState _state = {};
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	uint16_t* _paletteRam = nullptr;

public:
	PceVce(Emulator* emu, PceConsole* console);
	~PceVce();

	uint16_t GetScanlineCount() { return _state.ScanlineCount; }
	uint8_t GetClockDivider() { return _state.ClockDivider; }
	bool IsGrayscale() { return _state.Grayscale; }
	
	uint16_t GetPalette(uint16_t addr) { return _paletteRam[addr]; }
	PceVceState& GetState() { return _state; }

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};
