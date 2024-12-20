#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class SmsController : public BaseControlDevice
{
private:
	uint32_t _turboSpeed = 0;

protected:
	string GetKeyNames() override
	{
		return "UDLR12P";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::A, keyMapping.A);
			SetPressedState(Buttons::B, keyMapping.B);
			SetPressedState(Buttons::Up, keyMapping.Up);
			SetPressedState(Buttons::Down, keyMapping.Down);
			SetPressedState(Buttons::Left, keyMapping.Left);
			SetPressedState(Buttons::Right, keyMapping.Right);
			
			SetPressedState(Buttons::Pause, keyMapping.Start);

			uint8_t turboFreq = 1 << (4 - _turboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::A, keyMapping.TurboA);
				SetPressedState(Buttons::B, keyMapping.TurboB);
			}

			if(!_emu->GetSettings()->GetSmsConfig().AllowInvalidInput) {
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
	}

public:
	enum Buttons { Up = 0, Down, Left, Right, B, A, Pause };

	SmsController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::SmsController, port, keyMappings)
	{
		_turboSpeed = keyMappings.TurboSpeed;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		uint8_t value = 0xFF;
		if(addr == 0) {
			if(_port == 0) {
				value ^= IsPressed(Buttons::Up) ? 0x01 : 0;
				value ^= IsPressed(Buttons::Down) ? 0x02 : 0;
				value ^= IsPressed(Buttons::Left) ? 0x04 : 0;
				value ^= IsPressed(Buttons::Right) ? 0x08 : 0;
				value ^= IsPressed(Buttons::B) ? 0x10 : 0;
				value ^= IsPressed(Buttons::A) ? 0x20 : 0;
			} else if(_port == 1) {
				value ^= IsPressed(Buttons::Up) ? 0x40 : 0;
				value ^= IsPressed(Buttons::Down) ? 0x80 : 0;
			}
		} else {
			if(_port == 1) {
				value ^= IsPressed(Buttons::Left) ? 0x01 : 0;
				value ^= IsPressed(Buttons::Right) ? 0x02 : 0;
				value ^= IsPressed(Buttons::B) ? 0x04 : 0;
				value ^= IsPressed(Buttons::A) ? 0x08 : 0;
			}
		}
		return value;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(35, 14);

		hud.DrawButton(5, 3, 3, 3, IsPressed(Buttons::Up));
		hud.DrawButton(5, 9, 3, 3, IsPressed(Buttons::Down));
		hud.DrawButton(2, 6, 3, 3, IsPressed(Buttons::Left));
		hud.DrawButton(8, 6, 3, 3, IsPressed(Buttons::Right));
		hud.DrawButton(5, 6, 3, 3, false);

		hud.DrawButton(25, 7, 3, 3, IsPressed(Buttons::B));
		hud.DrawButton(30, 7, 3, 3, IsPressed(Buttons::A));

		if(_port == 0) {
			hud.DrawButton(15, 9, 5, 2, IsPressed(Buttons::Pause));
		}

		hud.DrawNumber(_port + 1, 16, 2);
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "two", Buttons::A },
			{ "one", Buttons::B },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
			{ "pause", Buttons::Pause },
		};
	}
};