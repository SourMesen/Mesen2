#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class SuborKeyboard : public BaseControlDevice
{
private:
	uint8_t _row = 0;
	uint8_t _column = 0;
	bool _enabled = false;

protected:
	string GetKeyNames() override
	{
		return "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567891234567890120123456789edpmdmncdsasbemglrcpcsasbteeehidududlr123";
	}

	enum Buttons
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
		NumpadEnter, NumpadDot, NumpadPlus, NumpadMultiply, NumpadDivide, NumpadMinus, NumLock,
		Comma, Dot, SemiColon, Apostrophe,
		Slash, Backslash,
		Equal, Minus, Grave,
		LeftBracket, RightBracket,
		CapsLock, Pause,
		Ctrl, Shift, Alt,
		Space, Backspace, Tab, Esc, Enter,
		End, Home,
		Ins, Delete,
		PageUp, PageDown,
		Up, Down, Left, Right,
		Unknown1, Unknown2, Unknown3, None
	};

	Buttons _keyboardMatrix[104] = {
		Num4, G, F, C, F2, E, Num5, V,
		Num2, D, S, End, F1, W, Num3, X,
		Ins, Backspace, PageDown, Right, F8, PageUp, Delete, Home,
		Num9, I, L, Comma, F5, O, Num0, Dot,
		RightBracket, Enter, Up, Left, F7, LeftBracket, Backslash, Down,
		Q, CapsLock, Z, Tab, Esc, A, Num1, Ctrl,
		Num7, Y, K, M, F4, U, Num8, J,
		Minus, SemiColon, Apostrophe, Slash, F6, P, Equal, Shift,
		T, H, N, Space, F3, R, Num6, B,
		Numpad6, NumpadEnter, Numpad4, Numpad8, None, Unknown1, Unknown2, Unknown3,
		Alt, Numpad4, Numpad7, F11, F12, Numpad1, Numpad2, Numpad8,
		NumpadMinus, NumpadPlus, NumpadMultiply, Numpad9, F10, Numpad5, NumpadDivide, NumLock,
		Grave, Numpad6, Pause, Space, F9, Numpad3, NumpadDot, Numpad0
	};

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = 0; i < 99; i++) {
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}
		}
	}

	uint8_t GetActiveKeys(uint8_t row, uint8_t column)
	{
		uint8_t result = 0;
		uint32_t baseIndex = row * 8 + (column ? 4 : 0);
		for(int i = 0; i < 4; i++) {
			if(IsPressed(_keyboardMatrix[baseIndex + i])) {
				result |= (1 << i);
			}
		}

		if(row == 9 && column) {
			//This bit is used to indicate that this is an "extended" version of
			//the keyboard which has four more rows to read from (numpad, etc.)
			//Corresponds to row 9, bit 4
			result |= 0x01;
		}

		return result;
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_row); SV(_column); SV(_enabled);
	}

	void RefreshStateBuffer() override
	{
		_row = 0;
		_column = 0;
	}	

public:
	SuborKeyboard(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::SuborKeyboard, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			if(_enabled) {
				uint8_t value = ((~GetActiveKeys(_row, _column)) << 1) & 0x1E;
				return value;
			} else {
				return 0x1E;
			}
		}
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);

		uint8_t prevColumn = _column;
		_column = (value & 0x02) >> 1;
		_enabled = (value & 0x04) != 0;

		if(_enabled) {
			if(!_column && prevColumn) {
				_row = (_row + 1) % 13;
			}
		}
	}
};