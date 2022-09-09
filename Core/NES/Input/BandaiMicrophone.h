#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

class BandaiMicrophone : public BaseControlDevice
{
protected:
	enum Buttons { A, B, Microphone };

	string GetKeyNames() override
	{
		return "ABM";
	}

	void InternalSetStateFromInput() override
	{
		//Make sure the key bindings are properly updated (not ideal, but good enough)
		_keyMappings = _emu->GetSettings()->GetNesConfig().MapperInput.Keys.GetKeyMappingArray();

		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::A, keyMapping.CustomKeys[BandaiMicrophone::Buttons::A]);
			SetPressedState(Buttons::B, keyMapping.CustomKeys[BandaiMicrophone::Buttons::B]);
			if((_emu->GetFrameCount() % 2) == 0) {
				//Alternate between 1 and 0s (not sure if the game does anything with this data?)
				SetPressedState(Buttons::Microphone, keyMapping.CustomKeys[BandaiMicrophone::Buttons::Microphone]);
			}
		}
	}

public:
	BandaiMicrophone(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::BandaiMicrophone, BaseControlDevice::MapperInputPort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr >= 0x6000 && addr <= 0x7FFF) {
			return
				(IsPressed(Buttons::A) ? 0 : 0x01) |
				(IsPressed(Buttons::B) ? 0 : 0x02) |
				(IsPressed(Buttons::Microphone) ? 0x04 : 0);
		} else {
			return 0;
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}
};