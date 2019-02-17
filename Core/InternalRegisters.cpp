#include "stdafx.h"
#include "InternalRegisters.h"
#include "MessageManager.h"
#include "../Utilities/HexUtilities.h"

uint8_t InternalRegisters::Read(uint16_t addr)
{
	switch(addr) {
		case 0x4214: return (uint8_t)_divResult;
		case 0x4215: return (uint8_t)(_divResult >> 8);

		case 0x4216: return (uint8_t)_multOrRemainderResult;
		case 0x4217: return (uint8_t)(_multOrRemainderResult >> 8);

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

			//TODO
			//_autoJoypadRead = (value & 0x01) != 0;
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
