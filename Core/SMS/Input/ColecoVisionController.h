#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class ColecoVisionController : public BaseControlDevice
{
private:
	uint32_t _turboSpeed = 0;
	bool _modeSelect = false;

protected:
	string GetKeyNames() override
	{
		return "UDLRlr1234567890*#";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			for(int i = Buttons::Up; i <= Buttons::Pound; i++) {
				SetPressedState(i, keyMapping.CustomKeys[i]);
			}
		}
	}

	void RefreshStateBuffer() override
	{}

	static constexpr uint8_t _keypadLookup[12] = {
		0xD, 0x7, 0xC, 0x2, 0x3, 0xE, 0x5, 0x1, 0xB, 0xA, 0x9, 0x6
	};

public:
	enum Buttons { Up = 0, Down, Left, Right, L, R, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0, Star, Pound };

	ColecoVisionController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::ColecoVisionController, port, keyMappings)
	{
		_turboSpeed = keyMappings.TurboSpeed;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(addr == _port) {
			if(_modeSelect) {
				return (
					0x30 | 
					(IsPressed(Buttons::L) ? 0 : 0x40) |
					(IsPressed(Buttons::Up) ? 0 : 0x01) |
					(IsPressed(Buttons::Right) ? 0 : 0x02) |
					(IsPressed(Buttons::Down) ? 0 : 0x04) |
					(IsPressed(Buttons::Left) ? 0 : 0x08)
				);
			} else {
				uint8_t rightButton = IsPressed(Buttons::R) ? 0 : 0x40;

				for(int i = Buttons::Num1; i <= Buttons::Pound; i++) {
					if(IsPressed(i)) {
						return 0x30 | _keypadLookup[i - Buttons::Num1] | rightButton;
					}
				}

				return 0x30 | rightButton | 0xF;
			}
		}
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_modeSelect = value & 0x01;
	}

	void InternalDrawController(InputHud& hud) override
	{
		hud.DrawOutline(21, 31);

		hud.DrawButton(9, 3, 3, 3, IsPressed(Buttons::Up));
		hud.DrawButton(9, 9, 3, 3, IsPressed(Buttons::Down));
		hud.DrawButton(6, 6, 3, 3, IsPressed(Buttons::Left));
		hud.DrawButton(12, 6, 3, 3, IsPressed(Buttons::Right));
		hud.DrawButton(9, 6, 3, 3, false);

		hud.DrawButton(0, 10, 3, 3, IsPressed(Buttons::L));
		hud.DrawButton(18, 10, 3, 3, IsPressed(Buttons::R));

		hud.DrawButton(4, 14, 3, 3, IsPressed(Buttons::Num1));
		hud.DrawButton(9, 14, 3, 3, IsPressed(Buttons::Num2));
		hud.DrawButton(14, 14, 3, 3, IsPressed(Buttons::Num3));

		hud.DrawButton(4, 18, 3, 3, IsPressed(Buttons::Num4));
		hud.DrawButton(9, 18, 3, 3, IsPressed(Buttons::Num5));
		hud.DrawButton(14, 18, 3, 3, IsPressed(Buttons::Num6));

		hud.DrawButton(4, 22, 3, 3, IsPressed(Buttons::Num7));
		hud.DrawButton(9, 22, 3, 3, IsPressed(Buttons::Num8));
		hud.DrawButton(14, 22, 3, 3, IsPressed(Buttons::Num9));

		hud.DrawButton(4, 26, 3, 3, IsPressed(Buttons::Star));
		hud.DrawButton(9, 26, 3, 3, IsPressed(Buttons::Num0));
		hud.DrawButton(14, 26, 3, 3, IsPressed(Buttons::Pound));

		hud.DrawNumber(_port + 1, 16, 2);
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "zero", Buttons::Num0 },
			{ "one", Buttons::Num1 },
			{ "two", Buttons::Num2 },
			{ "three", Buttons::Num3 },
			{ "four", Buttons::Num4 },
			{ "five", Buttons::Num5 },
			{ "six", Buttons::Num6 },
			{ "seven", Buttons::Num7 },
			{ "eight", Buttons::Num8 },
			{ "nine", Buttons::Num9 },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
			{ "l", Buttons::L },
			{ "r", Buttons::R },
			{ "star", Buttons::Star },
			{ "pound", Buttons::Pound },
		};
	}
};