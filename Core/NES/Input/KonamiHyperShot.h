#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

class KonamiHyperShot : public BaseControlDevice
{
private:
	bool _enableP1 = true;
	bool _enableP2 = true;

protected:
	enum Buttons { Player1Run = 0, Player1Jump, Player2Run, Player2Jump };

	string GetKeyNames() override
	{
		return "RJrj";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Player1Jump, keyMapping.CustomKeys[Buttons::Player1Jump]);
			SetPressedState(Buttons::Player1Run, keyMapping.CustomKeys[Buttons::Player1Run]);
			SetPressedState(Buttons::Player2Jump, keyMapping.CustomKeys[Buttons::Player2Jump]);
			SetPressedState(Buttons::Player2Run, keyMapping.CustomKeys[Buttons::Player2Run]);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_enableP1); SV(_enableP2);
	}

public:
	KonamiHyperShot(Emulator* emu, KeyMappingSet keys) : BaseControlDevice(emu, ControllerType::KonamiHyperShot, BaseControlDevice::ExpDevicePort, keys)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if(addr == 0x4017) {
			if(_enableP1) {
				output |= IsPressed(KonamiHyperShot::Buttons::Player1Jump) ? 0x02 : 0;
				output |= IsPressed(KonamiHyperShot::Buttons::Player1Run) ? 0x04 : 0;
			}
			if(_enableP2) {
				output |= IsPressed(KonamiHyperShot::Buttons::Player2Jump) ? 0x08 : 0;
				output |= IsPressed(KonamiHyperShot::Buttons::Player2Run) ? 0x10 : 0;
			}
		}
		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_enableP2 = (value & 0x02) == 0;
		_enableP1 = (value & 0x04) == 0;
	}
};