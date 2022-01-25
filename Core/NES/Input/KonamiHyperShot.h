#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

class KonamiHyperShot : public BaseControlDevice
{
private:
	bool _enableP1 = true;
	bool _enableP2 = true;
	uint32_t _p1TurboSpeed;
	uint32_t _p2TurboSpeed;
	vector<KeyMapping> _p2KeyMappings;

protected:
	enum Buttons { Player1Run = 0, Player1Jump, Player2Run, Player2Jump };

	string GetKeyNames() override
	{
		return "RJrj";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Player1Jump, keyMapping.A);
			SetPressedState(Buttons::Player1Run, keyMapping.B);

			uint8_t turboFreq = 1 << (4 - _p1TurboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::Player1Jump, keyMapping.TurboA);
				SetPressedState(Buttons::Player1Run, keyMapping.TurboB);
			}
		}

		for(KeyMapping& keyMapping : _p2KeyMappings) {
			SetPressedState(Buttons::Player2Jump, keyMapping.A);
			SetPressedState(Buttons::Player2Run, keyMapping.B);

			uint8_t turboFreq = 1 << (4 - _p2TurboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::Player2Jump, keyMapping.TurboA);
				SetPressedState(Buttons::Player2Run, keyMapping.TurboB);
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		s.Stream(_enableP1, _enableP2);
	}

public:
	KonamiHyperShot(Emulator* emu, KeyMappingSet p1, KeyMappingSet p2) : BaseControlDevice(emu, ControllerType::KonamiHyperShot, BaseControlDevice::ExpDevicePort, p1)
	{
		_p1TurboSpeed = p1.TurboSpeed;
		_p2TurboSpeed = p2.TurboSpeed;
		_p2KeyMappings = p2.GetKeyMappingArray();
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