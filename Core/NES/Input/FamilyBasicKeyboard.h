#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class FamilyBasicKeyboard : public BaseControlDevice
{
private:
	uint8_t _row = 0;
	uint8_t _column = 0;
	bool _enabled = false;

	const uint32_t _keyMatrix[72] = {
		F8, Return, LeftBracket, RightBracket,
		Kana, RightShift, Yen, Stop,
		F7, AtSign, Colon, SemiColon,
		Underscore, Slash, Minus, Caret,
		F6, O, L, K,
		Dot, Comma, P, Num0,
		F5, I, U, J,
		M, N, Num9, Num8,
		F4, Y, G, H,
		B, V, Num7, Num6,
		F3, T, R, D,
		F, C, Num5, Num4,
		F2, W, S, A,
		X, Z, E, Num3,
		F1, Esc, Q, Ctrl,
		LeftShift, Grph, Num1, Num2,
		ClrHome, Up, Right, Left,
		Down, Space, Del, Ins
	};

	enum Buttons
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		Return, Space, Del, Ins, Esc,
		Ctrl, RightShift, LeftShift,
		RightBracket, LeftBracket,
		Up, Down, Left, Right,
		Dot, Comma, Colon, SemiColon, Underscore, Slash, Minus, Caret,
		F1, F2, F3, F4, F5, F6, F7, F8,
		Yen, Stop, AtSign, Grph, ClrHome, Kana
	};

	string GetKeyNames() override
	{
		return "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789rsdiecSs[]udlrd,:;_/-^12345678ysagck";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = 0; i < 72; i++) {
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}
		}
	}

	uint8_t GetActiveKeys(uint8_t row, uint8_t column)
	{
		if(row == 9) {
			//10th row has no keys
			return 0;
		}

		uint8_t result = 0;
		uint32_t baseIndex = row * 8 + (column ? 4 : 0);
		for(int i = 0; i < 4; i++) {
			if(IsPressed(_keyMatrix[baseIndex + i])) {
				result |= 0x10;
			}
			result >>= 1;
		}
		return result;
	}

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_row); SV(_column); SV(_enabled);
	}

public:
	FamilyBasicKeyboard(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::FamilyBasicKeyboard, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			if(_enabled) {
				return ((~GetActiveKeys(_row, _column)) << 1) & 0x1E;
			} else {
				return 0;
			}
		}
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		uint8_t prevColumn = _column;
		_column = (value & 0x02) >> 1;
		if(!_column && prevColumn) {
			//"Incrementing the row from the (keyless) 10th row will cause it to wrap back to the first row."
			_row = (_row + 1) % 10;
		}

		if(value & 0x01) {
			_row = 0;
		}

		_enabled = (value & 0x04) != 0;
	}
};