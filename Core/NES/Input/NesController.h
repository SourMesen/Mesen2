#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class NesController : public BaseControlDevice
{
private:
	bool _microphoneEnabled = false;
	uint32_t _turboSpeed = 0;
	uint8_t _stateBuffer = 0;

protected:
	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_stateBuffer); SV(_microphoneEnabled);
	}

	uint8_t GetControllerStateBuffer()
	{
		return _stateBuffer;
	}

	string GetKeyNames() override
	{
		string keys = "UDLRSsBA";
		if(_microphoneEnabled) {
			keys += "M";
		}
		return keys;
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::A, keyMapping.A);
			SetPressedState(Buttons::B, keyMapping.B);
			SetPressedState(Buttons::Start, keyMapping.Start);
			SetPressedState(Buttons::Select, keyMapping.Select);
			SetPressedState(Buttons::Up, keyMapping.Up);
			SetPressedState(Buttons::Down, keyMapping.Down);
			SetPressedState(Buttons::Left, keyMapping.Left);
			SetPressedState(Buttons::Right, keyMapping.Right);

			uint8_t turboFreq = 1 << (4 - _turboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::A, keyMapping.TurboA);
				SetPressedState(Buttons::B, keyMapping.TurboB);
			}

			if(_microphoneEnabled && (_emu->GetFrameCount() % 3) == 0) {
				SetPressedState(Buttons::Microphone, keyMapping.GenericKey1);
			}

			if(!_emu->GetSettings()->GetNesConfig().AllowInvalidInput) {
				//If both U+D or L+R are pressed at the same time, act as if neither is pressed
				if(IsPressed(Buttons::Up) && IsPressed(Buttons::Down)) {
					ClearBit(Buttons::Down);
					ClearBit(Buttons::Up);
				}
				if(IsPressed(Buttons::Left) && IsPressed(Buttons::Right)) {
					ClearBit(Buttons::Left);
					ClearBit(Buttons::Right);
				}
			}
		}
	}

	void RefreshStateBuffer() override
	{
		_stateBuffer = ToByte();
	}

public:
	enum Buttons { Up = 0, Down, Left, Right, Start, Select, B, A, Microphone };

	NesController(Emulator* emu, ControllerType type, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, type, port, keyMappings)
	{
		_turboSpeed = keyMappings.TurboSpeed;
		_microphoneEnabled = port == 1 && type == ControllerType::FamicomControllerP2;
	}
	
	uint8_t ToByte()
	{
		//"Button status for each controller is returned as an 8-bit report in the following order: A, B, Select, Start, Up, Down, Left, Right."
		return
			(uint8_t)IsPressed(Buttons::A) |
			((uint8_t)IsPressed(Buttons::B) << 1) |
			((uint8_t)IsPressed(Buttons::Select) << 2) |
			((uint8_t)IsPressed(Buttons::Start) << 3) |
			((uint8_t)IsPressed(Buttons::Up) << 4) |
			((uint8_t)IsPressed(Buttons::Down) << 5) |
			((uint8_t)IsPressed(Buttons::Left) << 6) |
			((uint8_t)IsPressed(Buttons::Right) << 7);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t output = 0;

		if(IsCurrentPort(addr)) {
			StrobeProcessRead();
			
			output = _stateBuffer & 0x01;
			_stateBuffer >>= 1;

			//"All subsequent reads will return D=1 on an authentic controller but may return D=0 on third party controllers."
			_stateBuffer |= 0x80;
		}

		if(addr == 0x4016 && IsPressed(NesController::Buttons::Microphone)) {
			output |= 0x04;
		}

		return output;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		StrobeProcessWrite(value);
	}

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(35, 14);

		hud.DrawButton(5, 3, 3, 3, IsPressed(Buttons::Up));
		hud.DrawButton(5, 9, 3, 3, IsPressed(Buttons::Down));
		hud.DrawButton(2, 6, 3, 3, IsPressed(Buttons::Left));
		hud.DrawButton(8, 6, 3, 3, IsPressed(Buttons::Right));
		hud.DrawButton(5, 6, 3, 3, false);

		hud.DrawButton(30, 7, 3, 3, IsPressed(Buttons::A));
		hud.DrawButton(25, 7, 3, 3, IsPressed(Buttons::B));

		hud.DrawButton(13, 9, 4, 2, IsPressed(Buttons::Select));
		hud.DrawButton(18, 9, 4, 2, IsPressed(Buttons::Start));

		hud.DrawNumber(hud.GetControllerIndex() + 1, 16, 2);
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "a", Buttons::A },
			{ "b", Buttons::B },
			{ "start", Buttons::Start },
			{ "select", Buttons::Select },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
		};
	}
};