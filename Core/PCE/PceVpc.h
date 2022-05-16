#pragma once
#include "stdafx.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"

class PceVdc;
class PceVce;
class PceConsole;
class Emulator;

class PceVpc
{
public:
	static constexpr uint16_t SpritePixelFlag = 0x8000;
	static constexpr uint16_t TransparentPixelFlag = 0x4000;

private:
	PceVdc* _vdc1 = nullptr;
	PceVdc* _vdc2 = nullptr;
	PceVce* _vce = nullptr;
	Emulator* _emu = nullptr;
	PceConsole* _console = nullptr;
	bool _hasIrqVdc1 = false;
	bool _hasIrqVdc2 = false;

	uint16_t* _outBuffer[2] = {};
	uint16_t* _currentOutBuffer = nullptr;

	uint8_t _rowVceClockDivider[2][PceConstants::ScreenHeight] = {};
	uint8_t* _currentClockDividers = nullptr;

	PceVpcState _state = {};

	void SetPriorityConfig(PceVpcPixelWindow wnd, uint8_t value);
	void UpdateIrqState();

public:
	PceVpc(Emulator* emu, PceConsole* console, PceVce* vce);
	~PceVpc();
	
	void ConnectVdc(PceVdc* vdc1, PceVdc* vdc2);

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void StVdcWrite(uint16_t addr, uint8_t value);

	void Exec();
	void DrawScanline();
	void ProcessScanlineStart(PceVdc* vdc, uint16_t scanline);
	void ProcessScanlineEnd(PceVdc* vdc, uint16_t scanline, uint16_t* rowBuffer);
	void SendFrame(PceVdc* vdc);

	void SetIrq(PceVdc* vdc);
	void ClearIrq(PceVdc* vdc);

	PceVpcState GetState() { return _state; }

	uint16_t* GetScreenBuffer() { return _currentOutBuffer; }
	uint16_t* GetPreviousScreenBuffer() { return _currentOutBuffer == _outBuffer[0] ? _outBuffer[1] : _outBuffer[0]; }
	uint8_t* GetRowClockDividers() { return _currentClockDividers; }
	uint8_t* GetPreviousRowClockDividers() { return _currentClockDividers == _rowVceClockDivider[0] ? _rowVceClockDivider[1] : _rowVceClockDivider[0]; }
};