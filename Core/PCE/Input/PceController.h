#pragma once
#include "pch.h"
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
		return "UDLRSr12";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::I, keyMapping.A);
			SetPressedState(Buttons::II, keyMapping.B);
			SetPressedState(Buttons::Run, keyMapping.Start);
			SetPressedState(Buttons::Select, keyMapping.Select);
			SetPressedState(Buttons::Up, keyMapping.Up);
			SetPressedState(Buttons::Down, keyMapping.Down);
			SetPressedState(Buttons::Left, keyMapping.Left);
			SetPressedState(Buttons::Right, keyMapping.Right);

			uint8_t turboFreq = 1 << (4 - _turboSpeed);
			bool turboOn = (uint8_t)(_emu->GetFrameCount() % turboFreq) < turboFreq / 2;
			if(turboOn) {
				SetPressedState(Buttons::I, keyMapping.TurboA);
				SetPressedState(Buttons::II, keyMapping.TurboB);
			}
		}

		PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();
		if(cfg.PreventSelectRunReset && IsPressed(Buttons::Run) && IsPressed(Buttons::Select)) {
			ClearBit(Buttons::Run);
			ClearBit(Buttons::Select);
		}

		if(!_emu->GetSettings()->GetPcEngineConfig().AllowInvalidInput) {
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

	void RefreshStateBuffer() override
	{
	}

	void Serialize(Serializer& s) override
	{
		BaseControlDevice::Serialize(s);
		SV(_disableInput);
		SV(_selectDPad);
	}

public:
	enum Buttons { Up = 0, Down, Left, Right, Select, Run, I, II };

	PceController(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::PceController, port, keyMappings)
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
			result &= ~(IsPressed(PceController::I) ? 0x01 : 0);
			result &= ~(IsPressed(PceController::II) ? 0x02 : 0);
			result &= ~(IsPressed(PceController::Select) ? 0x04 : 0);
			result &= ~(IsPressed(PceController::Run) ? 0x08 : 0);
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

		hud.DrawButton(30, 7, 3, 3, IsPressed(Buttons::I));
		hud.DrawButton(25, 7, 3, 3, IsPressed(Buttons::II));

		hud.DrawButton(13, 9, 4, 2, IsPressed(Buttons::Select));
		hud.DrawButton(18, 9, 4, 2, IsPressed(Buttons::Run));

		hud.DrawNumber(hud.GetControllerIndex() + 1, 16, 2);
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "i", Buttons::I },
			{ "ii", Buttons::II },
			{ "run", Buttons::Run },
			{ "select", Buttons::Select },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
		};
	}
};