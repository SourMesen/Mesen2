#pragma once
#include "pch.h"
#include "WS/WsConsole.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class WsController : public BaseControlDevice
{
private:
	WsConsole* _console = nullptr;
	vector<KeyMapping> _verticalMappings;
	uint32_t _turboSpeed = 0;

protected:
	string GetKeyNames() override
	{
		return "UDLRudlrSsBA";
	}

	void InternalSetStateFromInput() override
	{
		vector<KeyMapping>& keyMappings = _console->IsVerticalMode() ? _verticalMappings : _keyMappings;
		for(KeyMapping& keyMapping : keyMappings) {
			SetPressedState(Buttons::A, keyMapping.A);
			SetPressedState(Buttons::B, keyMapping.B);
			SetPressedState(Buttons::Sound, keyMapping.GenericKey1);
			SetPressedState(Buttons::Start, keyMapping.Start);
			SetPressedState(Buttons::Up, keyMapping.Up);
			SetPressedState(Buttons::Down, keyMapping.Down);
			SetPressedState(Buttons::Left, keyMapping.Left);
			SetPressedState(Buttons::Right, keyMapping.Right);

			SetPressedState(Buttons::Up2, keyMapping.U);
			SetPressedState(Buttons::Down2, keyMapping.D);
			SetPressedState(Buttons::Left2, keyMapping.L);
			SetPressedState(Buttons::Right2, keyMapping.R);

			uint8_t turboFreq = 1 << (4 - _turboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::A, keyMapping.TurboA);
				SetPressedState(Buttons::B, keyMapping.TurboB);
			}
		}
	}

	void RefreshStateBuffer() override
	{}

public:
	enum Buttons { Up = 0, Down, Left, Right, Up2, Down2, Left2, Right2, Sound, Start, B, A };

	WsController(Emulator* emu, WsConsole* console, uint8_t port, KeyMappingSet horizontalMappings, KeyMappingSet verticalMappings) : BaseControlDevice(emu, ControllerType::WsController, port, horizontalMappings)
	{
		//TODOWS turbo support
		_verticalMappings = verticalMappings.GetKeyMappingArray();
		_console = console;
		_turboSpeed = horizontalMappings.TurboSpeed;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
	}

	void InternalDrawController(InputHud& hud) override
	{
		if(_console->IsVerticalMode()) {
			hud.DrawOutline(28, 31);

			hud.DrawButton(20, 20, 3, 3, IsPressed(Buttons::Right));
			hud.DrawButton(20, 26, 3, 3, IsPressed(Buttons::Left));
			hud.DrawButton(17, 23, 3, 3, IsPressed(Buttons::Up));
			hud.DrawButton(23, 23, 3, 3, IsPressed(Buttons::Down));

			hud.DrawButton(5, 20, 3, 3, IsPressed(Buttons::Right2));
			hud.DrawButton(5, 26, 3, 3, IsPressed(Buttons::Left2));
			hud.DrawButton(2, 23, 3, 3, IsPressed(Buttons::Up2));
			hud.DrawButton(8, 23, 3, 3, IsPressed(Buttons::Down2));

			hud.DrawButton(23, 6, 3, 3, IsPressed(Buttons::B));
			hud.DrawButton(20, 3, 3, 3, IsPressed(Buttons::A));

			hud.DrawButton(21, 11, 2, 3, IsPressed(Buttons::Start));
			hud.DrawButton(21, 15, 2, 3, IsPressed(Buttons::Sound));

			hud.DrawNumber(_port + 1, 13, 2);
		} else {
			hud.DrawOutline(35, 24);

			hud.DrawButton(5, 2, 3, 3, IsPressed(Buttons::Up2));
			hud.DrawButton(5, 8, 3, 3, IsPressed(Buttons::Down2));
			hud.DrawButton(2, 5, 3, 3, IsPressed(Buttons::Left2));
			hud.DrawButton(8, 5, 3, 3, IsPressed(Buttons::Right2));

			hud.DrawButton(5, 13, 3, 3, IsPressed(Buttons::Up));
			hud.DrawButton(5, 19, 3, 3, IsPressed(Buttons::Down));
			hud.DrawButton(2, 16, 3, 3, IsPressed(Buttons::Left));
			hud.DrawButton(8, 16, 3, 3, IsPressed(Buttons::Right));

			hud.DrawButton(25, 19, 3, 3, IsPressed(Buttons::B));
			hud.DrawButton(29, 16, 3, 3, IsPressed(Buttons::A));

			hud.DrawButton(14, 17, 4, 2, IsPressed(Buttons::Sound));
			hud.DrawButton(19, 17, 4, 2, IsPressed(Buttons::Start));

			hud.DrawNumber(_port + 1, 16, 2);
		}
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "a", Buttons::A },
			{ "b", Buttons::B },
			{ "sound", Buttons::Sound },
			{ "start", Buttons::Start },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
			{ "up2", Buttons::Up2 },
			{ "down2", Buttons::Down2 },
			{ "left2", Buttons::Left2 },
			{ "right2", Buttons::Right2 },
		};
	}
};