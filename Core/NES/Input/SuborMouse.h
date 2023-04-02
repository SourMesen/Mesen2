#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/KeyManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Utilities/Serializer.h"

class SuborMouse : public BaseControlDevice
{
private:
	uint32_t _stateBuffer = 0;
	uint8_t _packetBytes[3] = {};
	uint8_t _packetPos = 0;
	uint8_t _packetSize = 1;

protected:
	bool HasCoordinates() override { return true; }
	enum Buttons { Left = 0, Right };

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SVArray(_packetBytes, 3);
		SV(_stateBuffer); SV(_packetPos); SV(_packetSize);
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::Left, KeyManager::IsKeyPressed(keyMapping.CustomKeys[0]));
			SetPressedState(Buttons::Right, KeyManager::IsKeyPressed(keyMapping.CustomKeys[1]));
		}
		SetMovement(KeyManager::GetMouseMovement(_emu, _emu->GetSettings()->GetInputConfig().MouseSensitivity));
	}

public:
	SuborMouse(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::SuborMouse, port, keyMappings)
	{
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;
		if((addr == 0x4016 && (_port & 0x01) == 0) || (addr == 0x4017 && (_port & 0x01) == 1)) {
			StrobeProcessRead();
			output = (_stateBuffer & 0x80) >> 7;
			if(_port >= 2) {
				output <<= 1;
			}
			_stateBuffer <<= 1;
		}
		return output;
	}

	void RefreshStateBuffer() override
	{
		if(_packetPos < _packetSize - 1) {
			//3-byte packet is not done yet, move to next byte
			_packetPos++;
			_stateBuffer = _packetBytes[_packetPos];
			return;
		}

		MouseMovement mov = GetMovement();
		
		uint32_t upFlag = mov.dy < 0 ? 0x80 : 0;
		uint32_t leftFlag = mov.dx < 0 ? 0x80 : 0;

		mov.dx = std::min<int16_t>(std::abs(mov.dx), 31);
		mov.dy = std::min<int16_t>(std::abs(mov.dy), 31);

		if(mov.dx <= 1 && mov.dy <= 1) {
			//1-byte packet
			_packetBytes[0] =
				(IsPressed(SuborMouse::Buttons::Left) ? 0x80 : 0) |
				(IsPressed(SuborMouse::Buttons::Right) ? 0x40 : 0) |
				(leftFlag && mov.dx ? 0x30 : (mov.dx ? 0x10 : 0)) |
				(upFlag && mov.dy ? 0x0C : (mov.dy ? 0x04 : 0));
			_packetBytes[1] = 0;
			_packetBytes[2] = 0;

			_packetSize = 1;
		} else {
			//3-byte packet
			_packetBytes[0] =
				(IsPressed(SuborMouse::Buttons::Left) ? 0x80 : 0) |
				(IsPressed(SuborMouse::Buttons::Right) ? 0x40 : 0) |
				(leftFlag ? 0x20 : 0) |
				(mov.dx & 0x10) |
				(upFlag ? 0x08 : 0) |
				((mov.dy & 0x10) >> 2) |
				0x01;

			_packetBytes[1] = ((mov.dx & 0x0F) << 2) | 0x02;
			_packetBytes[2] = ((mov.dy & 0x0F) << 2) | 0x03;

			_packetSize = 3;
		}

		_packetPos = 0;
		_stateBuffer = _packetBytes[0];
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "xOffset", BaseControlDevice::DeviceXCoordButtonId, true },
			{ "yOffset", BaseControlDevice::DeviceYCoordButtonId, true },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right }
		};
	}
};