#include "stdafx.h"
#include "InternalRegisters.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "ControlManager.h"
#include "MemoryManager.h"
#include "MessageManager.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/HexUtilities.h"

InternalRegisters::InternalRegisters(shared_ptr<Console> console)
{
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_ppu = _console->GetPpu().get();
	Reset();

	//Power on values
	_horizontalTimer = 0x1FF;
	_verticalTimer = 0x1FF;
	_multOperand1 = 0xFF;
	_dividend = 0xFFFF;
	_ioPortOutput = 0xFF;
}

void InternalRegisters::Reset()
{
	_enableAutoJoypadRead = false;
	_enableNmi = false;
	_enableHorizontalIrq = false;
	_enableVerticalIrq = false;
	_nmiFlag = false;
	_irqLevel = false;
	_needIrq = false;
}

void InternalRegisters::ProcessAutoJoypadRead()
{
	if(!_enableAutoJoypadRead) {
		return;
	}

	shared_ptr<ControlManager> controlManager = _console->GetControlManager();

	controlManager->Write(0x4016, 1);
	controlManager->Write(0x4016, 0);

	for(int i = 0; i < 4; i++) {
		_controllerData[i] = 0;
	}

	for(int i = 0; i < 16; i++) {
		uint8_t port1 = controlManager->Read(0x4016);
		uint8_t port2 = controlManager->Read(0x4017);

		_controllerData[0] <<= 1;
		_controllerData[1] <<= 1;
		_controllerData[2] <<= 1;
		_controllerData[3] <<= 1;

		_controllerData[0] |= (port1 & 0x01);
		_controllerData[1] |= (port2 & 0x01);
		_controllerData[2] |= (port1 & 0x02) >> 1;
		_controllerData[3] |= (port2 & 0x02) >> 1;
	}
}

void InternalRegisters::ProcessIrqCounters()
{
	if(_needIrq) {
		_needIrq = false;
		_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
	}

	bool irqLevel = (
		(_enableHorizontalIrq || _enableVerticalIrq) &&
		(!_enableHorizontalIrq || (_horizontalTimer <= 339 && (_ppu->GetCycle() == _horizontalTimer) && (_ppu->GetLastScanline() != _ppu->GetScanline() || _horizontalTimer < 339))) &&
		(!_enableVerticalIrq || _ppu->GetScanline() == _verticalTimer)
	);

	if(!_irqLevel && irqLevel) {
		_needIrq = true;
	}
	_irqLevel = irqLevel;
}

uint8_t InternalRegisters::GetIoPortOutput()
{
	return _ioPortOutput;
}

void InternalRegisters::SetNmiFlag(bool nmiFlag)
{
	_nmiFlag = nmiFlag;
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
			uint16_t vblankStart = _ppu->GetVblankStart();
			//TODO TIMING (set/clear timing)
			return (
				(scanline >= vblankStart ? 0x80 : 0) |
				((hClock >= 1*4 && hClock <= 274*4) ? 0 : 0x40) |
				((_enableAutoJoypadRead && scanline >= vblankStart && scanline <= vblankStart + 2) ? 0x01 : 0) | //Auto joypad read in progress
				(_memoryManager->GetOpenBus() & 0x3E)
			);
		}

		case 0x4213:
			//TODO  RDIO - Programmable I/O port (in-port)
			return 0;
						 
		case 0x4214: return (uint8_t)_divResult;
		case 0x4215: return (uint8_t)(_divResult >> 8);

		case 0x4216: return (uint8_t)_multOrRemainderResult;
		case 0x4217: return (uint8_t)(_multOrRemainderResult >> 8);

		case 0x4218: case 0x421A: case 0x421C: case 0x421E: 
			return (uint8_t)_controllerData[((addr & 0x0E) - 8) >> 1];
		
		case 0x4219: case 0x421B: case 0x421D: case 0x421F:
			return (uint8_t)(_controllerData[((addr & 0x0E) - 8) >> 1] >> 8);

		default:
			MessageManager::Log("[Debug] Unimplemented register read: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void InternalRegisters::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4200:
			if((value & 0x30) == 0x20 && !_enableVerticalIrq && _ppu->GetScanline() == _verticalTimer) {
				//When enabling vertical irqs, if the current scanline matches the target scanline, set the irq flag right away
				_console->GetCpu()->SetIrqSource(IrqSource::Ppu);
			}

			if((value & 0x80) && !_enableNmi && _nmiFlag) {
				_console->GetCpu()->SetNmiFlag();
			}

			_enableNmi = (value & 0x80) != 0;
			_enableVerticalIrq = (value & 0x20) != 0;
			_enableHorizontalIrq = (value & 0x10) != 0;

			if(!_enableHorizontalIrq && !_enableVerticalIrq) {
				//TODO VERIFY
				_console->GetCpu()->ClearIrqSource(IrqSource::Ppu);
			}
			
			_enableAutoJoypadRead = (value & 0x01) != 0;
			break;

		case 0x4201:
			//TODO WRIO - Programmable I/O port (out-port)
			if((_ioPortOutput & 0x80) && !(value & 0x80)) {
				_ppu->LatchLocationValues();
			}
			_ioPortOutput = value;
			break;

		case 0x4202: _multOperand1 = value; break;
		case 0x4203:
			_multOperand2 = value;
			_multOrRemainderResult = _multOperand1 * _multOperand2;
			break;

		case 0x4204: _dividend = (_dividend & 0xFF00) | value; break;
		case 0x4205: _dividend = (_dividend & 0xFF) | (value << 8); break;
		case 0x4206:
			_divisor = value;
			if(_divisor == 0) {
				//"Division by 0 gives a quotient of $FFFF and a remainder of C."
				_divResult = 0xFFFF;
				_multOrRemainderResult = _dividend;
			} else {
				_divResult = _dividend / _divisor;
				_multOrRemainderResult = _dividend % _divisor;
			}
			break;

		case 0x4207: _horizontalTimer = (_horizontalTimer & 0x100) | value; break;
		case 0x4208: _horizontalTimer = (_horizontalTimer & 0xFF) | ((value & 0x01) << 8); break;

		case 0x4209: _verticalTimer = (_verticalTimer & 0x100) | value; break;
		case 0x420A: _verticalTimer = (_verticalTimer & 0xFF) | ((value & 0x01) << 8); break;

		case 0x420D: _enableFastRom = (value & 0x01) != 0; break;

		default:
			MessageManager::Log("[Debug] Unimplemented register write: " + HexUtilities::ToHex(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void InternalRegisters::Serialize(Serializer &s)
{
	s.Stream(
		_multOperand1, _multOperand2, _multOrRemainderResult, _dividend, _divisor, _divResult, _enableAutoJoypadRead,
		_enableFastRom, _nmiFlag, _enableNmi, _enableHorizontalIrq, _enableVerticalIrq, _horizontalTimer,
		_verticalTimer, _ioPortOutput, _controllerData[0], _controllerData[1], _controllerData[2], _controllerData[3],
		_irqLevel, _needIrq
	);
}
