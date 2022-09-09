#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/KeyManager.h"
#include "Utilities/Serializer.h"

class OekaKidsTablet : public BaseControlDevice
{
private:
	bool _shift = false;
	uint32_t _stateBuffer = 0;

protected:
	enum Buttons { Click, Touch };
	bool HasCoordinates() override { return true; }

	string GetKeyNames() override
	{
		return "CT";
	}

	void InternalSetStateFromInput() override
	{
		MousePosition pos = KeyManager::GetMousePosition();
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Click, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));
			SetPressedState(Buttons::Touch, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));			
		}
		SetPressedState(Buttons::Touch, pos.Y >= 48);
		SetCoordinates(pos);
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_shift); SV(_stateBuffer);
	}

public:
	OekaKidsTablet(Emulator* emu, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::OekaKidsTablet, BaseControlDevice::ExpDevicePort, keyMappings)
	{
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == 0x4017) {
			if(_strobe) {
				if(_shift) {
					return (_stateBuffer & 0x40000) ? 0x00 : 0x08;
				} else {
					return 0x04;
				}
			} else {
				return 0x00;
			}
		}

		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_strobe = (value & 0x01) == 0x01;
		bool shift = ((value >> 1) & 0x01) == 0x01;

		if(_strobe) {
			if(!_shift && shift) {
				_stateBuffer <<= 1;
			}
			_shift = shift;
		} else {
			MousePosition pos = GetCoordinates();

			uint8_t xPosition = (uint8_t)((double)std::max(0, pos.X + 8) / 256.0 * 240);
			uint8_t yPosition = (uint8_t)((double)std::max(0, pos.Y - 14) / 240.0 * 256);

			_stateBuffer = (xPosition << 10) | (yPosition << 2) | (IsPressed(OekaKidsTablet::Buttons::Touch) ? 0x02 : 0x00) | (IsPressed(OekaKidsTablet::Buttons::Click) ? 0x01 : 0x00);
		}
	}
};