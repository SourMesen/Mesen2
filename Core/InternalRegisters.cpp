#include "stdafx.h"
#include "InternalRegisters.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "ControlManager.h"
#include "MemoryManager.h"
#include "MessageManager.h"
#include "InternalRegisterTypes.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/HexUtilities.h"

InternalRegisters::InternalRegisters()
{
}

void InternalRegisters::Initialize(Console* console)
{
	_aluMulDiv.Initialize(console->GetCpu().get());
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_ppu = _console->GetPpu().get();
	Reset();

	//Power on values
	_state = {};
	_state.HorizontalTimer = 0x1FF;
	_state.VerticalTimer = 0x1FF;
	_state.IoPortOutput = 0xFF;
}

void InternalRegisters::Reset()
{
	_state.EnableAutoJoypadRead = false;
	_state.EnableNmi = false;
	_state.EnableHorizontalIrq = false;
	_state.EnableVerticalIrq = false;
	_nmiFlag = false;
	_irqLevel = false;
	_needIrq = false;
}

void InternalRegisters::ProcessAutoJoypadRead()
{
	if(!_state.EnableAutoJoypadRead) {
		return;
	}

	shared_ptr<ControlManager> controlManager = _console->GetControlManager();

	controlManager->Write(0x4016, 1);
	controlManager->Write(0x4016, 0);

	for(int i = 0; i < 4; i++) {
		_state.ControllerData[i] = 0;
	}

	for(int i = 0; i < 16; i++) {
		uint8_t port1 = controlManager->Read(0x4016);
		uint8_t port2 = controlManager->Read(0x4017);

		_state.ControllerData[0] <<= 1;
		_state.ControllerData[1] <<= 1;
		_state.ControllerData[2] <<= 1;
		_state.ControllerData[3] <<= 1;

		_state.ControllerData[0] |= (port1 & 0x01);
		_state.ControllerData[1] |= (port2 & 0x01);
		_state.ControllerData[2] |= (port1 & 0x02) >> 1;
		_state.ControllerData[3] |= (port2 & 0x02) >> 1;
	}
}

uint8_t InternalRegisters::GetIoPortOutput()
{
	return _state.IoPortOutput;
}

void InternalRegisters::SetNmiFlag(bool nmiFlag)
{
	_nmiFlag = nmiFlag;
}

uint8_t InternalRegisters::Peek(uint16_t addr)
{
	switch(addr) {
		case 0x4210: return (_nmiFlag ? 0x80 : 0) | 0x02;
		case 0x4211: return (_console->GetCpu()->CheckIrqSource(IrqSource::Ppu) ? 0x80 : 0);
		default: return Read(addr);
	}
}

uint8_t InternalRegisters::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210: {
			uint8_t value = 
				(_nmiFlag ? 0x80 : 0) |
				0x02; //CPU revision

			_nmiFlag = false;
			return value | (_memoryManager->GetOpenBus() & 0x70);
		}

		case 0x4211: {
			uint8_t value = (_console->GetCpu()->CheckIrqSource(IrqSource::Ppu) ? 0x80 : 0);
			_console->GetCpu()->ClearIrqSource(IrqSource::Ppu);
			return value | (_memoryManager->GetOpenBus() & 0x7F);
		}

		case 0x4212: {
			uint16_t hClock = _memoryManager->GetHClock();
			uint16_t scanline = _ppu->GetScanline();
			uint16_t nmiScanline = _ppu->GetNmiScanline();
			//TODO TIMING (set/clear timing)
			return (
				(scanline >= nmiScanline ? 0x80 : 0) |
				((hClock >= 1*4 && hClock <= 274*4) ? 0 : 0x40) |
				((_state.EnableAutoJoypadRead && scanline >= nmiScanline && scanline <= nmiScanline + 2) ? 0x01 : 0) | //Auto joypad read in progress
				(_memoryManager->GetOpenBus() & 0x3E)
			);
		}

		case 0x4213:
			//TODO  RDIO - Programmable I/O port (in-port)
			return 0;
						 
		case 0x4214:
		case 0x4215:
		case 0x4216:
		case 0x4217: 
			return _aluMulDiv.Read(addr);

		case 0x4218: return (uint8_t)_state.ControllerData[0];
		case 0x4219: return (uint8_t)(_state.ControllerData[0] >> 8);
		case 0x421A: return (uint8_t)_state.ControllerData[1];
		case 0x421B: return (uint8_t)(_state.ControllerData[1] >> 8);
		case 0x421C: return (uint8_t)_state.ControllerData[2];
		case 0x421D: return (uint8_t)(_state.ControllerData[2] >> 8);
		case 0x421E: return (uint8_t)_state.ControllerData[3];
		case 0x421F: return (uint8_t)(_state.ControllerData[3] >> 8);
		
		default:
			LogDebug("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			return _memoryManager->GetOpenBus();
	}
}

void InternalRegisters::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4200:
			if((value & 0x30) == 0x20 && !_state.EnableVerticalIrq && _ppu->GetRealScanline() == _state.VerticalTimer) {
				//When enabling vertical irqs, if the current scanline matches the target scanline, set the irq flag right away
				_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
			}

			if((value & 0x80) && !_state.EnableNmi && _nmiFlag) {
				_console->GetCpu()->SetNmiFlag();
			}

			_state.EnableNmi = (value & 0x80) != 0;
			_state.EnableVerticalIrq = (value & 0x20) != 0;
			_state.EnableHorizontalIrq = (value & 0x10) != 0;

			if(!_state.EnableHorizontalIrq && !_state.EnableVerticalIrq) {
				//TODO VERIFY
				_console->GetCpu()->ClearIrqSource(IrqSource::Ppu);
			}
			
			_state.EnableAutoJoypadRead = (value & 0x01) != 0;
			break;

		case 0x4201:
			//TODO WRIO - Programmable I/O port (out-port)
			if((_state.IoPortOutput & 0x80) && !(value & 0x80)) {
				_ppu->LatchLocationValues();
			}
			_state.IoPortOutput = value;
			break;

		case 0x4202:
		case 0x4203:
		case 0x4204:
		case 0x4205:
		case 0x4206:
			_aluMulDiv.Write(addr, value);
			break;

		case 0x4207: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0x100) | value; 
			ProcessIrqCounters();
			break;

		case 0x4208: 
			_state.HorizontalTimer = (_state.HorizontalTimer & 0xFF) | ((value & 0x01) << 8); 
			ProcessIrqCounters();
			break;

		case 0x4209: 
			_state.VerticalTimer = (_state.VerticalTimer & 0x100) | value; 

			//Calling this here fixes flashing issue in "Shin Nihon Pro Wrestling Kounin - '95 Tokyo Dome Battle 7"
			//The write to change from scanline 16 to 17 occurs between both ProcessIrqCounter calls, which causes the IRQ
			//line to always be high (since the previous check is on scanline 16, and the next check on scanline 17)
			ProcessIrqCounters();
			break;

		case 0x420A: 
			_state.VerticalTimer = (_state.VerticalTimer & 0xFF) | ((value & 0x01) << 8);
			ProcessIrqCounters();
			break;

		case 0x420D: _state.EnableFastRom = (value & 0x01) != 0; break;

		default:
			LogDebug("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

InternalRegisterState InternalRegisters::GetState()
{
	return _state;
}

AluState InternalRegisters::GetAluState()
{
	return _aluMulDiv.GetState();
}

void InternalRegisters::Serialize(Serializer &s)
{
	s.Stream(
		_state.EnableFastRom, _nmiFlag, _state.EnableNmi, _state.EnableHorizontalIrq, _state.EnableVerticalIrq, _state.HorizontalTimer,
		_state.VerticalTimer, _state.IoPortOutput, _state.ControllerData[0], _state.ControllerData[1], _state.ControllerData[2], _state.ControllerData[3],
		_irqLevel, _needIrq, _state.EnableAutoJoypadRead
	);

	s.Stream(&_aluMulDiv);
}
