#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

class Emulator;

class PartyTap : public BaseControlDevice
{
private:
	uint8_t _stateBuffer = 0;
	uint8_t _readCount = 0;
	bool _enabled = false;

protected:
	enum Buttons { B1 = 0, B2, B3, B4, B5, B6 };

	string GetKeyNames() override
	{
		return "123456";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = 0; i < 6; i++) {
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_stateBuffer); SV(_readCount); SV(_enabled);
	}

public:
	PartyTap(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::PartyTap, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			StrobeProcessRead();
			if(_readCount < 2) {
				uint8_t value = (_stateBuffer & 0x7) << 2;
				_stateBuffer >>= 3;
				_readCount++;
				return value;
			} else {
				//"After 1st/2nd reads,	a detection value can be read : $4017 & $1C == $14"
				return 0x14;
			}
		}
		return 0;
	}

	void RefreshStateBuffer() override
	{
		_readCount = 0;
		_stateBuffer =
			(IsPressed(PartyTap::Buttons::B1) ? 1 : 0) |
			(IsPressed(PartyTap::Buttons::B2) ? 2 : 0) |
			(IsPressed(PartyTap::Buttons::B3) ? 4 : 0) |
			(IsPressed(PartyTap::Buttons::B4) ? 8 : 0) |
			(IsPressed(PartyTap::Buttons::B5) ? 16 : 0) |
			(IsPressed(PartyTap::Buttons::B6) ? 32 : 0);
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}
};