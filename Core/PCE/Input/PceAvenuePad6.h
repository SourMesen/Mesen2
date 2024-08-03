#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/InputHud.h"
#include "Utilities/Serializer.h"

class PceAvenuePad6 : public BaseControlDevice
{
private:
	uint32_t _turboSpeed = 0;
	bool _disableInput = false;
	bool _selectDPad = false;
	bool _selectExtraButtons = false;

protected:
	string GetKeyNames() override
	{
		return "UDLRSr123456";
	}

	void InternalSetStateFromInput() override
	{
		for(KeyMapping& keyMapping : _keyMappings) {
			SetPressedState(Buttons::I, keyMapping.A);
			SetPressedState(Buttons::II, keyMapping.B);
			SetPressedState(Buttons::III, keyMapping.X);
			SetPressedState(Buttons::IV, keyMapping.Y);
			SetPressedState(Buttons::V, keyMapping.L);
			SetPressedState(Buttons::VI, keyMapping.R);
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
				SetPressedState(Buttons::III, keyMapping.TurboX);
				SetPressedState(Buttons::IV, keyMapping.TurboY);
				SetPressedState(Buttons::V, keyMapping.TurboL);
				SetPressedState(Buttons::VI, keyMapping.TurboR);
			}
		}

		PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();
		if(cfg.PreventSelectRunReset && IsPressed(Buttons::Run) && IsPressed(Buttons::Select)) {
			ClearBit(Buttons::Run);
			ClearBit(Buttons::Select);
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
		SV(_selectExtraButtons);
	}

public:
	enum Buttons { Up = 0, Down, Left, Right, Select, Run, I, II, III, IV, V, VI };

	PceAvenuePad6(Emulator* emu, uint8_t port, KeyMappingSet keyMappings) : BaseControlDevice(emu, ControllerType::PceAvenuePad6, port, keyMappings)
	{
		_turboSpeed = keyMappings.TurboSpeed;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		if(_disableInput) {
			return 0x0F;
		}

		uint8_t result = 0x0F;
		if(_selectExtraButtons) {
			if(_selectDPad) {
				result = 0;
			} else {
				result &= ~(IsPressed(PceAvenuePad6::III) ? 0x01 : 0);
				result &= ~(IsPressed(PceAvenuePad6::IV) ? 0x02 : 0);
				result &= ~(IsPressed(PceAvenuePad6::V) ? 0x04 : 0);
				result &= ~(IsPressed(PceAvenuePad6::VI) ? 0x08 : 0);
			}
		} else {
			if(_selectDPad) {
				result &= ~(IsPressed(PceAvenuePad6::Up) ? 0x01 : 0);
				result &= ~(IsPressed(PceAvenuePad6::Right) ? 0x02 : 0);
				result &= ~(IsPressed(PceAvenuePad6::Down) ? 0x04 : 0);
				result &= ~(IsPressed(PceAvenuePad6::Left) ? 0x08 : 0);
			} else {
				result &= ~(IsPressed(PceAvenuePad6::I) ? 0x01 : 0);
				result &= ~(IsPressed(PceAvenuePad6::II) ? 0x02 : 0);
				result &= ~(IsPressed(PceAvenuePad6::Select) ? 0x04 : 0);
				result &= ~(IsPressed(PceAvenuePad6::Run) ? 0x08 : 0);
			}
		}

		return result;
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		bool disableInput = (value & 0x02) != 0;
		if(disableInput && !_disableInput) {
			_selectExtraButtons = !_selectExtraButtons;
		}
		_disableInput = disableInput;
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

		hud.DrawButton(22, 8, 3, 3, IsPressed(Buttons::III));
		hud.DrawButton(26, 8, 3, 3, IsPressed(Buttons::II));
		hud.DrawButton(30, 8, 3, 3, IsPressed(Buttons::I));

		hud.DrawButton(22, 3, 3, 3, IsPressed(Buttons::IV));
		hud.DrawButton(26, 3, 3, 3, IsPressed(Buttons::V));
		hud.DrawButton(30, 3, 3, 3, IsPressed(Buttons::VI));

		hud.DrawButton(12, 9, 4, 2, IsPressed(Buttons::Select));
		hud.DrawButton(17, 9, 4, 2, IsPressed(Buttons::Run));

		hud.DrawNumber(hud.GetControllerIndex() + 1, 15, 2);
	}

	vector<DeviceButtonName> GetKeyNameAssociations() override
	{
		return {
			{ "i", Buttons::I },
			{ "ii", Buttons::II },
			{ "iii", Buttons::III },
			{ "iv", Buttons::IV },
			{ "v", Buttons::V },
			{ "vi", Buttons::VI },
			{ "run", Buttons::Run },
			{ "select", Buttons::Select },
			{ "up", Buttons::Up },
			{ "down", Buttons::Down },
			{ "left", Buttons::Left },
			{ "right", Buttons::Right },
		};
	}
};