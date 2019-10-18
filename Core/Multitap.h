#pragma once
#include "stdafx.h"
#include "BaseControlDevice.h"
#include "InternalRegisters.h"
#include "../Utilities/Serializer.h"

class Multitap : public BaseControlDevice
{
private:
	enum Buttons { A = 0, B, X, Y, L, R, Select, Start, Up, Down, Left, Right };
	static constexpr int ButtonCount = 12;

	vector<KeyMapping> _mappings[4];
	uint16_t _stateBuffer[4] = {};
	InternalRegisters *_internalRegs = nullptr;

protected:
	string GetKeyNames() override
	{
		//Repeat key names 4x, once for each controller
		return "ABXYLRSTUDLR:ABXYLRSTUDLR:ABXYLRSTUDLR:ABXYLRSTUDLR";
	}

	void InternalSetStateFromInput() override
	{
		for(int i = 0; i < 4; i++) {
			int offset = Multitap::ButtonCount * i;
			for(KeyMapping keyMapping : _mappings[i]) {
				SetPressedState(Buttons::A + offset, keyMapping.A);
				SetPressedState(Buttons::B + offset, keyMapping.B);
				SetPressedState(Buttons::X + offset, keyMapping.X);
				SetPressedState(Buttons::Y + offset, keyMapping.Y);
				SetPressedState(Buttons::L + offset, keyMapping.L);
				SetPressedState(Buttons::R + offset, keyMapping.R);
				SetPressedState(Buttons::Start + offset, keyMapping.Start);
				SetPressedState(Buttons::Select + offset, keyMapping.Select);
				SetPressedState(Buttons::Up + offset, keyMapping.Up);
				SetPressedState(Buttons::Down + offset, keyMapping.Down);
				SetPressedState(Buttons::Left + offset, keyMapping.Left);
				SetPressedState(Buttons::Right + offset, keyMapping.Right);
			}
		}
	}

	uint16_t ToByte(uint8_t port)
	{
		//"A Super NES controller returns a 16-bit report in a similar order: B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R, then four 0 bits."

		int offset = port * Multitap::ButtonCount;

		return
			(uint8_t)IsPressed(Buttons::B + offset) |
			((uint8_t)IsPressed(Buttons::Y + offset) << 1) |
			((uint8_t)IsPressed(Buttons::Select + offset) << 2) |
			((uint8_t)IsPressed(Buttons::Start + offset) << 3) |
			((uint8_t)IsPressed(Buttons::Up + offset) << 4) |
			((uint8_t)IsPressed(Buttons::Down + offset) << 5) |
			((uint8_t)IsPressed(Buttons::Left + offset) << 6) |
			((uint8_t)IsPressed(Buttons::Right + offset) << 7) |
			((uint8_t)IsPressed(Buttons::A + offset) << 8) |
			((uint8_t)IsPressed(Buttons::X + offset) << 9) |
			((uint8_t)IsPressed(Buttons::L + offset) << 10) |
			((uint8_t)IsPressed(Buttons::R + offset) << 11);
	}

	void Serialize(Serializer &s) override
	{
		BaseControlDevice::Serialize(s);
		s.Stream(_stateBuffer[0], _stateBuffer[1], _stateBuffer[2], _stateBuffer[3]);
	}

	void RefreshStateBuffer() override
	{
		for(int i = 0; i < 4; i++) {
			_stateBuffer[i] = ToByte(i);
		}
	}

public:
	Multitap(Console* console, uint8_t port, KeyMappingSet keyMappings1, KeyMappingSet keyMappings2, KeyMappingSet keyMappings3, KeyMappingSet keyMappings4) : BaseControlDevice(console, port, keyMappings1)
	{
		_mappings[0] = keyMappings1.GetKeyMappingArray();
		_mappings[1] = keyMappings2.GetKeyMappingArray();
		_mappings[2] = keyMappings3.GetKeyMappingArray();
		_mappings[3] = keyMappings4.GetKeyMappingArray();
		_internalRegs = console->GetInternalRegisters().get();
	}

	ControllerType GetControllerType() override
	{
		return ControllerType::Multitap;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t selectBit = 0x80 >> ((_port == 0) ? 1 : 0);
		uint8_t portSelect = (_internalRegs->GetIoPortOutput() & selectBit) ? 0 : 2;
		uint8_t output = 0;

		if(IsCurrentPort(addr)) {
			StrobeProcessRead();

			output = _stateBuffer[portSelect] & 0x01;
			output |= (_stateBuffer[portSelect + 1] & 0x01) << 1;  //P3 & P5 are reported in bit 1

			if(_strobe) {
				//Bit 1 is always set when strobe is high
				output |= 0x02;
			}

			_stateBuffer[portSelect] >>= 1;
			_stateBuffer[portSelect + 1] >>= 1;

			//"All subsequent reads will return D=1 on an authentic controller but may return D=0 on third party controllers."
			_stateBuffer[portSelect] |= 0x8000;
			_stateBuffer[portSelect + 1] |= 0x8000;
		}

		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x4016) {
			StrobeProcessWrite(value);
		}
	}
};