#pragma once
#include "stdafx.h"
#include "Shared/Emulator.h"
#include "PCE/PceTypes.h"

class PceConsole;

class PcePpu
{
private:
	PcePpuState _state = {};
	Emulator* _emu;
	PceConsole* _console;
	uint16_t* _vram;
	uint16_t* _paletteRam;
	uint16_t* _spriteRam;

	uint16_t* _outBuffer;
	uint32_t _rowWidth = 256;

	template<uint16_t bitMask = 0xFFFF>
	void UpdateReg(uint16_t& reg, uint8_t value, bool msb)
	{
		if(msb) {
			reg = ((reg & 0xFF) | (value << 8)) & bitMask;
		} else {
			reg = ((reg & 0xFF00) | value) & bitMask;
		}
	}

	void LoadReadBuffer();
	void DrawScanline(bool drawOverscan);
	void SendFrame();

public:
	PcePpu(Emulator* emu, PceConsole* console);
	~PcePpu();

	PcePpuState& GetState();
	uint16_t GetCycle() { return _state.Cycle; }
	uint16_t GetScanline() { return _state.Scanline; }

	void Exec();

	void ProcessEndOfVisibleFrame();

	uint8_t ReadVdc(uint16_t addr);
	void WriteVdc(uint16_t addr, uint8_t value);

	uint8_t ReadVce(uint16_t addr);
	void WriteVce(uint16_t addr, uint8_t value);
};
