#pragma once
#include "stdafx.h"
#include "Shared/Emulator.h"
#include "PCE/PceTypes.h"

class PceConsole;

class PcePpu
{
private:
	PcePpuState _state = {};
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	uint16_t* _vram = nullptr;
	uint16_t* _paletteRam = nullptr;
	uint16_t* _spriteRam = nullptr;

	uint16_t* _outBuffer[2] = {};
	uint16_t* _currentOutBuffer = nullptr;
	uint32_t _screenWidth = 256;
	uint16_t _nextEvent = 0;

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
	uint32_t GetCurrentScreenWidth();
	void ChangeResolution();
	void SendFrame();

	void UpdateFrameTimings();

	__declspec(noinline) void CheckRcrScanlineValue();
	__declspec(noinline) void LatchScrollValues();
	__declspec(noinline) void ProcessEndOfScanline();
	__declspec(noinline) void ProcessEndOfVisibleFrame();
	__declspec(noinline) void ProcessSatbTransfer();
	__declspec(noinline) void ProcessEvent();

public:
	PcePpu(Emulator* emu, PceConsole* console);
	~PcePpu();

	PcePpuState& GetState();
	uint16_t* GetScreenBuffer();
	uint16_t* GetPreviousScreenBuffer();
	uint16_t GetScreenWidth();

	uint16_t GetHClock() { return _state.HClock; }
	uint16_t GetScanline() { return _state.Scanline; }
	uint16_t GetFrameCount() { return _state.FrameCount; }

	void Exec();

	uint8_t ReadVdc(uint16_t addr);
	void WriteVdc(uint16_t addr, uint8_t value);

	uint8_t ReadVce(uint16_t addr);
	void WriteVce(uint16_t addr, uint8_t value);
};
