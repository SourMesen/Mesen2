#include "stdafx.h"
#include "InternalRegisters.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "ControlManager.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"

InternalRegisters::InternalRegisters(shared_ptr<Console> console)
{
	_console = console;
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

void InternalRegisters::SetNmiFlag(bool nmiFlag)
{
	_nmiFlag = nmiFlag;
}

uint8_t InternalRegisters::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4210: {
			//open bus implementation here is needed to pass CPUPHL test
			uint8_t value = (_nmiFlag ? 0x80 : 0) | ((addr >> 8) & 0x70);
			_nmiFlag = false;
			return value;
		}

		case 0x4211: {
			uint8_t value = (_console->GetCpu()->CheckIrqSource(IrqSource::Ppu) ? 0x80 : 0) | ((addr >> 8) & 0x7F);
			_console->GetCpu()->ClearIrqSource(IrqSource::Ppu);
			return value;
		}

		case 0x4212: {
			PpuState state = _console->GetPpu()->GetState();
			return (
				(state.Scanline >= 225 ? 0x80 : 0) |
				((state.Cycle >= 0x121 || state.Cycle <= 0x15) ? 0x40 : 0) |
				((_enableAutoJoypadRead && state.Scanline >= 225 && state.Scanline <= 227) ? 0x01 : 0) //Auto joypad read in progress
			);
		}

		case 0x4214: return (uint8_t)_divResult;
		case 0x4215: return (uint8_t)(_divResult >> 8);

		case 0x4216: return (uint8_t)_multOrRemainderResult;
		case 0x4217: return (uint8_t)(_multOrRemainderResult >> 8);

		case 0x4218: case 0x421A: case 0x421C: case 0x421E: 
			return (uint8_t)_controllerData[((addr & 0x0E) - 8) >> 1];
		
		case 0x4219: case 0x421B: case 0x421D: case 0x421F:
			return (uint8_t)(_controllerData[((addr & 0x0E) - 8) >> 1] >> 8);

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register read: " + HexUtilities::ToHex(addr));
			return 0;
	}
}

void InternalRegisters::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0x4200:
			_enableNmi = (value & 0x80) != 0;
			_enableVerticalIrq = (value & 0x20) != 0;
			_enableHorizontalIrq = (value & 0x10) != 0;

			_enableAutoJoypadRead = (value & 0x01) != 0;
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

		default:
			MessageManager::DisplayMessage("Debug", "Unimplemented register write: " + HexUtilities::ToHex(addr));
			break;
	}
}
