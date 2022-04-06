#pragma once
#include "stdafx.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class PceController : public BaseControlDevice
{
private:
	uint32_t _turboSpeed = 0;
	bool _disableInput = false;
	bool _selectDPad = false;

protected:
	string GetKeyNames() override
	{
		return "UDLRSsBA";
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
		}
	}

	void RefreshStateBuffer() override
	{
	}

public:
	enum Buttons { Up = 0, Down, Left, Right, Start, Select, B, A };

	PceController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::GameboyController, port, keyMappings)
	{
		_turboSpeed = keyMappings.TurboSpeed;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(_disableInput) {
			return 0;
		}

		uint8_t result = 0x0F;
		if(_selectDPad) {
			result &= ~(IsPressed(PceController::Up) ? 0x01 : 0);
			result &= ~(IsPressed(PceController::Right) ? 0x02 : 0);
			result &= ~(IsPressed(PceController::Down) ? 0x04 : 0);
			result &= ~(IsPressed(PceController::Left) ? 0x08 : 0);
		} else {
			result &= ~(IsPressed(PceController::A) ? 0x01 : 0);
			result &= ~(IsPressed(PceController::B) ? 0x02 : 0);
			result &= ~(IsPressed(PceController::Select) ? 0x04 : 0);
			result &= ~(IsPressed(PceController::Start) ? 0x08 : 0);
		}

		return result;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_disableInput = (value & 0x02) != 0;
		_selectDPad = (value & 0x01) != 0;
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

		hud.DrawNumber(_port + 1, 16, 2);
	}
};