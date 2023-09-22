#pragma once
#include "pch.h"
#include "PCE/PceTypes.h"
#include "PCE/PceConstants.h"
#include "PCE/PceVdc.h"
#include "Utilities/ISerializable.h"

class PceVce;
class PceConsole;
class Emulator;

class PceVpc final : public ISerializable
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

	uint16_t* _outBuffer[2] = {};
	uint16_t* _currentOutBuffer = nullptr;
	uint16_t _xStart = 0;

	Timer _frameSkipTimer;
	bool _skipRender = false;

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

	__forceinline void Exec() { _vdc1->Exec(); }
	__forceinline void ExecSuperGrafx() { _vdc2->Exec(); _vdc1->Exec(); }

	void DrawScanline();
	void ProcessStartFrame();
	void ProcessScanlineStart(PceVdc* vdc, uint16_t scanline);
	void ProcessScanline();
	void ProcessScanlineEnd(PceVdc* vdc, uint16_t scanline, uint16_t* rowBuffer);
	void SendFrame(PceVdc* vdc);
	
	void DebugSendFrame();

	void SetIrq(PceVdc* vdc);
	void ClearIrq(PceVdc* vdc);
	
	bool IsSkipRenderEnabled() { return _skipRender; }

	PceVpcState GetState() { return _state; }

	uint16_t* GetScreenBuffer() { return _currentOutBuffer; }
	uint16_t* GetPreviousScreenBuffer() { return _currentOutBuffer == _outBuffer[0] ? _outBuffer[1] : _outBuffer[0]; }

	void Serialize(Serializer& s) override;
};